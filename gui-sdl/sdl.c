#include "sdl.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dt.h"
#include "keyboard.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

SDL_Cursor* current_cursor = NULL;

int sdl_init(int w, int h) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("DrawTermSDL",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h,
                              SDL_WINDOW_RESIZABLE);
    SDL_Log("%s %s:%d", __PRETTY_FUNCTION__ , __FILE__ , __LINE__);
    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 2;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        window = NULL;
        return 3;
    }

    current_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    if(current_cursor == NULL) {
        SDL_Log("SDL_CreateSystemCursor (normal) Error: %s", SDL_GetError());
    }
    SDL_SetCursor(current_cursor);

    return 0;
}

void sdl_get_size(int *x, int *y) {
    SDL_GetWindowSize(window,x,y);
}
void sdl_cursor_set(unsigned char *data,unsigned char *mask, int x, int y) {
    SDL_Cursor *c = SDL_CreateCursor(data, mask, 16, 16, x, y);
    if(c) {
        if(current_cursor) {
            SDL_FreeCursor(current_cursor);
        }
        current_cursor = c;
        SDL_SetCursor(current_cursor);
    }
    SDL_ShowCursor(SDL_ENABLE);
}
void sdl_cursor_move(int x, int y){
    SDL_WarpMouseInWindow(window, x,y);
}
void sdl_update(unsigned char * argb32, int x, int y, int w, int h) {
    // update with null is to free the current surface, clear the screen ;-)
    SDL_Log("%s ... %p @%i,%i  (%i,%i)",__PRETTY_FUNCTION__, argb32 ,x, y, w, h);
    int sx, sy;

    // 'clear screen'
    if (argb32 == NULL)  {
        SDL_Log("... argb is NULL, destroying main texture");
        SDL_DestroyTexture(texture);
        texture = NULL;
        return;
    }

    // no texture, create an appropriately sized one
    if(!texture) {
        SDL_Log("... main texture is null, creating it");
        sdl_get_size(&sx,&sy);
        texture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    sx,
                                    sy);

        if (!texture) {
            SDL_Log("SDL_CreateTexture(texture) error: %s", SDL_GetError());
            sdl_update(NULL, 0, 0, 0, 0);
            return;
        }
        SDL_Log("... main texture is %p",texture);
    }

    if(texture) { // (texture exists)
        sdl_get_size(&sx,&sy);

        SDL_Log("... main texture exists and screen is: %d, %d",sx,sy);
        if ( x == 0 && y == 0 && w == sx && h == sy) { // full screen update
            SDL_Log("   ... full screen update");

            unsigned char *p;
            int pitch;
            if (SDL_LockTexture(texture, NULL, (void **) &p, &pitch)) {
                SDL_Log("SDL_LockTexture(texture) Error: %s", SDL_GetError());
                sdl_update(NULL, 0, 0, 0, 0);
                return;
            }
            // just copy the entire backbuffer
            memcpy(p, argb32, pitch * h);
            SDL_UnlockTexture(texture);
        }

        if(1){
            SDL_Log("   ... partial update @%d,%d (%d x %d)",x,y,w,h);

            // partial update
            SDL_Texture *update = SDL_CreateTexture(renderer,
                                                    SDL_PIXELFORMAT_ARGB8888,
                                                    SDL_TEXTUREACCESS_STREAMING,
                                                    w,
                                                    h);
            if (!update) {
                SDL_Log("SDL_CreateTexture(update) error: %s", SDL_GetError());
                // probably need a full screen update
                return;
            }
            SDL_Log("copying update to texture: %p", update);
            unsigned char *p;
            int pitch;
            if (SDL_LockTexture(update, NULL, (void **) &p, &pitch)) {
                SDL_Log("SDL_LockTexture(update) Error: %s", SDL_GetError());
                // probably need a full screen update here too
                return;
            }
            //memcpy(p, argb32, pitch * h);
            for(int i=0;i<(pitch*h);i+=4) {
                *p = 0xFF; // alpha
                p++;
                *p = 0xFF; // red
                p++;
                *p = 0x00; // green
                p++;
                *p = 0xFF; // blue
                p++;
            }
            SDL_UnlockTexture(update);

            SDL_Log("copying update texture %p to rendertarget %p", update, texture);
            // now write update on texture
            SDL_SetRenderTarget(renderer, texture);
            SDL_Rect src_rect = {0,0,w,h};
            SDL_Rect dst_rect = {x,y,w,h};
            // might need to set 'SDL_SetTextureBlendMode (SDL_BLENDMODE_NONE)' here?
            if(SDL_RenderCopy(renderer, update, &src_rect, &dst_rect)) {
                SDL_Log("SDL_RenderCopy (update->texture) Error: %s", SDL_GetError());
                // trigger another fullscreen update?
                return;
            }
            // Detach Render Target
            SDL_SetRenderTarget(renderer, NULL);
            // destroy update
            SDL_DestroyTexture(update);
        }
        SDL_Log("... drawing to screen");
        // texture exists and (partial) screen has been copied to it
        // send texture to screen
        SDL_RenderClear(renderer);
        // might need SDL_RenderCopyEx to render upside down for mobile devices
        // i.e.	SDL_RenderCopyEx(renderer, texture, NULL, NULL, 0, NULL, SDL_FLIP_VERTICAL);
        if(SDL_RenderCopy(renderer, texture, NULL, NULL)) {
            SDL_Log("SDL_RenderCopy (texture->screen) Error: %s", SDL_GetError());
            // trigger fullscreen update?
            return;
        }
        SDL_RenderPresent(renderer);
    }
}

// This is a horrible hack, and we need to do some proper keymapping here
// Assuming the keyboard in front of me (UK Mac, except for 3/#)
// is generic enough so most people can function with it for now
int sdl_key_map(SDL_Event *e) {
    int c = e->key.keysym.sym;
    if(c == SDLK_RETURN)
        return '\n';

    int shifted = (e->key.keysym.mod == KMOD_LSHIFT || e->key.keysym.mod == KMOD_RSHIFT);
    int caps = e->key.keysym.mod == KMOD_CAPS;
    if (isalpha(c) & (shifted || caps)) {
        return toupper(c);
    }
    // punctuation on US/ISO format
    if (ispunct(c) & shifted) {
        switch(c) {
            case '-': return '_';
            case '=': return '+';
            case '[': return '{';
            case ']': return '}';
            case ';': return ':';
            case '\'': return '"';
            case '\\': return '|';
            case ',': return '<';
            case '.': return '>';
            case '/': return '?';
            case '`': return '~';
            default:
                break;
        }
    }
    // us number keymapping
    if (isdigit(c) & shifted) {
        switch (c) {
            case '0': return ')';
            case '1': return '!';
            case '2': return '@';
            case '3': return '#';
            case '4': return '$';
            case '5': return '%';
            case '6': return '^';
            case '7': return '&';
            case '8': return '*';
            case '9': return '(';
            default:
                break;
        }
    }
    return c;
}
int sdl_key_special(SDL_Event *e) {
    switch (e->key.keysym.sym) {
        // middle cluster
        case SDLK_HOME:        return Khome;
        case SDLK_UP:          return Kup;
        case SDLK_DOWN:        return Kdown;
        case SDLK_PAGEUP:      return Kpgup;
        case SDLK_PRINTSCREEN: return Kprint;  // ASSUMPTION
        case SDLK_LEFT:        return Kleft;
        case SDLK_RIGHT:       return Kright;
        case SDLK_PAGEDOWN:    return Kpgdown;
        case SDLK_INSERT:      return Kins;    // Also Help on some macs
        case SDLK_END:         return Kend;
        case SDLK_SCROLLLOCK:  return Kscroll; // ASSUMPTION

        // modifiers
        case SDLK_LALT:
        case SDLK_RALT:        return Kalt;
        // ^^ possibly: case SDLK_RALT: return Kaltgr;

        case SDLK_LSHIFT:
        case SDLK_RSHIFT:      return Kshift;
        case SDLK_LCTRL:
        case SDLK_RCTRL:       return Kctl;
        case SDLK_LGUI:
        case SDLK_RGUI:        return Kmod4;


        case SDLK_AUDIOPREV:   return Ksbwd;
        case SDLK_AUDIONEXT:   return Ksfwd;
        case SDLK_AUDIOSTOP:   return Kpause;
        case SDLK_VOLUMEDOWN:  return Kvoldn;
        case SDLK_VOLUMEUP:    return Kvolup;
        case SDLK_MUTE:                                 // ASSUMPTION
        case SDLK_AUDIOMUTE:   return Kmute;
        case SDLK_BRIGHTNESSDOWN: return Kbrtdn;
        case SDLK_BRIGHTNESSUP: return Kbrtup;
        case SDLK_PAUSE:       return Kbreak;
        // modifiers
        case SDLK_CAPSLOCK:     return Kcaps;
        case SDLK_NUMLOCKCLEAR: return Knum;  // CLEAR on some macs

        // Fn Keys.
        case SDLK_F1:  return KF|1;
        case SDLK_F2:  return KF|2;
        case SDLK_F3:  return KF|3;
        case SDLK_F4:  return KF|4;
        case SDLK_F5:  return KF|5;
        case SDLK_F6:  return KF|6;
        case SDLK_F7:  return KF|7;
        case SDLK_F8:  return KF|8;
        case SDLK_F9:  return KF|9;
        case SDLK_F10: return KF|10;
        case SDLK_F11: return KF|11;
        case SDLK_F12: return KF|12;

        // TODO: numpad
        default:
            break;
    }
    return 0;
}
void sdl_key_event(SDL_Event *e) {
    int key = e->key.keysym.sym;
    int down = e->type == SDL_KEYDOWN;

    if(key < 0x80) { // For all key symbols which are ascii
        key = sdl_key_map(e);
        post_keyboard(key,down);
    } else {
        key = sdl_key_special(e);
        if(key) {
            post_keyboard(key, down);
        }
    }
}
void sdl_mouse_wheel(SDL_Event *e) {
    int x,y,b,q;
    b = SDL_GetMouseState(&x, &y);
    q = (e->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1) * e->wheel.y;

    if(q != 0) {
        q = (q > 0 ? 8 : 16) | b;
        post_mouse(x, y, q, e->wheel.timestamp);
        post_mouse(x, y, b, e->wheel.timestamp);
    }
}
void sdl_mouse(SDL_Event *e) {
    int buttons = SDL_GetMouseState(NULL, NULL);

    post_mouse(e->button.x, e->button.y, buttons, e->button.timestamp);
}

void sdl_poll_event() {
    SDL_Event e;
    if(!SDL_WaitEventTimeout(&e,10000)) {
        return;
    }
    switch(e.type) {
        case SDL_QUIT:
            exit(0);
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            sdl_mouse(&e);
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            sdl_key_event(&e);
            break;
        case SDL_MOUSEWHEEL:
            sdl_mouse_wheel(&e);
            break;
        case SDL_WINDOWEVENT:
            if(e.window.event == SDL_WINDOWEVENT_RESIZED) {
                post_resize(e.window.data1, e.window.data2);
            }
            break;

        default:
            break;
    }
}

void sdl_loop() {
    sdl_init(1024,768); // TODO: get sensible defaults from command line

    start_cpu();

    while(1) {
        if(window != NULL) {
            sdl_poll_event();
        }
    }
    // if we ever get here, we should exit.
    exit(0);
}

int sdl_clipboard_write(const char *buf) {
    static char *empty = "";
    if (!buf) {
        buf = empty;
    }
    if (SDL_SetClipboardText(buf)) {
        SDL_Log("SDL_GetClipboardText Error: %s", SDL_GetError());
        return -1;
    }
    return 0;
}

char *sdl_clipboard_read() {
    static char *text;

    if (SDL_HasClipboardText()) {
        char *t = SDL_GetClipboardText();

        if (!t || (strlen(t) == 0)) {
            SDL_Log("SDL_GetClipboardText Error: %s", SDL_GetError());
            text = strdup("");
        } else {
            text = strdup(t);
        }
        return text;
    } else {
        return strdup("");
    }
}