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
SDL_Surface *surface=NULL;

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
    SDL_Log("%s %s:%d", __PRETTY_FUNCTION__ , __FILE__ , __LINE__);

    SDL_GetWindowSize(window,x,y);
}
void sdl_cursor_set(unsigned char *data,unsigned char *mask, int x, int y) {
    SDL_Log("%s %s:%d", __PRETTY_FUNCTION__ , __FILE__ , __LINE__);

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
    SDL_Log("%s %s:%d", __PRETTY_FUNCTION__ , __FILE__ , __LINE__);

    SDL_WarpMouseInWindow(window, x,y);
}
void sdl_update(unsigned char * argb32) {
    SDL_Log("%s %s:%d", __PRETTY_FUNCTION__ , __FILE__ , __LINE__);

    // update with null is to free the current surface
    if (argb32 == NULL) {
        if (surface) {
            SDL_FreeSurface(surface);
            surface = NULL;
        }
        return;
    }

    int w, h;
    sdl_get_size(&w, &h);

    // if surface is NULL (probably cleared for screen resize/initialisation)
    if (!surface) {

        int d, p;

        int depth = 32; //ARGB = 32
        int pitch = 4 * w; // pitch = 4 bytes times pixel width
        surface = SDL_CreateRGBSurfaceFrom((void *) argb32,
                                           w, h,
                                           depth, pitch,
                                           0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        if (!surface) {
            SDL_Log("Failed to create surface: %s", SDL_GetError());
            return;
        }
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        surface = NULL;
        return;
    }

    SDL_Rect destRect = {0, 0, w, h};
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(texture);

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
    SDL_Log("%s %s:%d", __PRETTY_FUNCTION__ , __FILE__ , __LINE__);

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

void sdl_poll_event() {
    SDL_Log("%s %s:%d", __PRETTY_FUNCTION__ , __FILE__ , __LINE__);

    SDL_Event e;
    if(!SDL_WaitEventTimeout(&e,10000)) {
        return;
    }
    int temp = 0;
    switch(e.type) {
        case SDL_QUIT:
            exit(0);
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            // SDL's Mousebuttons are reported in the same way to plan9
            temp = SDL_GetMouseState(NULL, NULL);
            post_mouse(e.button.x, e.button.y, temp, e.common.timestamp);
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            sdl_key_event(&e);
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
    SDL_Log("%s %s:%d", __PRETTY_FUNCTION__ , __FILE__ , __LINE__);

    start_cpu();

    while(1) {
        if(window != NULL) {
            sdl_poll_event();
        }
    }
    // if we ever get here, we should exit.
    exit(0);
}

