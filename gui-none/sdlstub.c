//
// Created by Simon Waite on 18/03/2024.
//

#include "sdlstub.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "none.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Cursor* normal_cursor = NULL;

int sdl_init(int w, int h) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("DrawTermSDL", 10, 10, w+10, h+10, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 2;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
//        SDL_Quit();
        return 3;
    }

    // mouse
    normal_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    if(normal_cursor == NULL) {
        SDL_Log("SDL_CreateSystemCursor (normal) Error: %s", SDL_GetError());
    }
    SDL_SetCursor(normal_cursor);
    return 0;
}


int sdl_write(int x, int y, unsigned char* rgb,int xmin, int xmax,int ymin,int ymax) {
    if(! window) {
        return 0;
    }

    // now copy rectangle to window, thanks.

    // ignore rectangle.
    int w = 1024;
    int h = 1024;
    SDL_Rect destRect = { 0, 0, w, h };

    int d = 24; // 24 bit
    int p = 3*1024; // pitch is the length of each scanline in bytes
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void*)rgb,w,h,d, p,0xFF0000, 0x00FF00, 0x0000FF, 0 );
    if (!surface) {
        SDL_Log("Failed to create surface: %s", SDL_GetError());
        return 10;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return 11;
    }
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);

    return 0;
}

void sdl_cursor_show(int flag) {
    int toggle = flag == 0 ? SDL_DISABLE : SDL_ENABLE;
    if(toggle != SDL_ShowCursor(toggle)) {
        SDL_Log("sdl_cursor_show: %d, Error: %s",flag,SDL_GetError());
    }
}
void sdl_cursor_move(int x, int y) {
    SDL_WarpMouseInWindow(NULL, x,y);
}


void tmp_understand_plain(SDL_Event *in, char *buf) {
    SDL_CommonEvent *e = (SDL_CommonEvent *)in;
    SDL_Log("%08u EVENT: '%s'", e->timestamp, buf);
}
void tmp_understand_display(SDL_Event *in, char *buf) {
    tmp_understand_plain(in,buf);
    SDL_DisplayEvent *e = (SDL_DisplayEvent *)in;
    SDL_Log("                SDL_DisplayEvent");
    SDL_Log("                e.display = %u", e->display);
    SDL_Log("                e.data1   = %u", e->data1);
}
void tmp_understand_window(SDL_Event *in, char *buf) {
    tmp_understand_plain(in,buf);
    SDL_WindowEvent *e = (SDL_WindowEvent *)in;
    char s[1024];
    switch (e->event) {
        case SDL_WINDOWEVENT_SHOWN:
            sprintf(s,"SDL_WINDOWEVENT_SHOWN");
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            sprintf(s,"SDL_WINDOWEVENT_HIDDEN");
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            sprintf(s,"SDL_WINDOWEVENT_EXPOSED");
            break;
        case SDL_WINDOWEVENT_MOVED:
            sprintf(s,"SDL_WINDOWEVENT_MOVED to data1,data1");
            break;
        case SDL_WINDOWEVENT_RESIZED:
            sprintf(s,"SDL_WINDOWEVENT_RESIZED to data1,data1");
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            sprintf(s,"SDL_WINDOWEVENT_RESIZED to data1,data1");
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            sprintf(s,"SDL_WINDOWEVENT_MINIMIZED");
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            sprintf(s,"SDL_WINDOWEVENT_MAXIMIZED");
            break;
        case SDL_WINDOWEVENT_RESTORED:
            sprintf(s,"SDL_WINDOWEVENT_RESTORED");
            break;
        case SDL_WINDOWEVENT_ENTER:
            sprintf(s,"SDL_WINDOWEVENT_ENTER");
            break;
        case SDL_WINDOWEVENT_LEAVE:
            sprintf(s,"SDL_WINDOWEVENT_LEAVE");
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            sprintf(s,"SDL_WINDOWEVENT_FOCUS_GAINED");
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            sprintf(s,"SDL_WINDOWEVENT_FOCUS_LOST");
            break;
        case SDL_WINDOWEVENT_CLOSE:
            sprintf(s,"SDL_WINDOWEVENT_CLOSE");
            break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
        case SDL_WINDOWEVENT_TAKE_FOCUS:
            sprintf(s,"SDL_WINDOWEVENT_TAKE_FOCUS");

            break;
        case SDL_WINDOWEVENT_HIT_TEST:
            sprintf(s,"SDL_WINDOWEVENT_HIT_TEST");
            break;
#endif
        default:
            sprintf(s,"Window %d got unknown event %d",
                    e->windowID, e->event);
            break;
    }

    SDL_Log("                SDL_WindowEvent");
    SDL_Log("                e.windowID= %u", e->windowID);
    SDL_Log("                e.event   = 0x%08x %s", e->event, s);
    SDL_Log("                e.data1   = %u", e->data1);
    SDL_Log("                e.data2   = %u", e->data2);
}
void tmp_understand_keyboard(SDL_Event *in, char *buf) {
    tmp_understand_plain(in,buf);
    SDL_KeyboardEvent *e = (SDL_KeyboardEvent *)in;
    char s[1024];
    SDL_Log("                SDL_KeyboardEvent");
    SDL_Log("                e.windowID= %u", e->windowID);
    switch(e->state) {
        case SDL_PRESSED: sprintf(buf, "Pressed"); break;
        case SDL_RELEASED: sprintf(buf, "Released"); break;
        default: sprintf(buf, "Unknown 0x%08x",e->state); break;
    }
    SDL_Log("                e.state   = (0x%x) %s", e->type, s);
    SDL_Log("                e.repeat  = (0x%x) %s", e->repeat,e->repeat == 0 ? "False" : "True");
    memset(s,0, sizeof(s));
    int n = e->keysym.mod;
    if(n == KMOD_NONE) { sprintf(s,"KMOD_NONE"); }
    else {
        if(n & KMOD_LSHIFT) strcat(s,"KMOD_LSHIFT ");
        if(n & KMOD_RSHIFT) strcat(s,"KMOD_RSHIFT ");
        if(n & KMOD_LCTRL)  strcat(s,"KMOD_LCTRL ");
        if(n & KMOD_RCTRL)  strcat(s,"KMOD_RCTRL ");
        if(n & KMOD_LALT)   strcat(s,"KMOD_LALT ");
        if(n & KMOD_RALT)   strcat(s,"KMOD_RALT ");
        if(n & KMOD_LGUI)   strcat(s,"KMOD_LGUI ");
        if(n & KMOD_RGUI)   strcat(s,"KMOD_RGUI ");
        if(n & KMOD_NUM)    strcat(s,"KMOD_NUM ");
        if(n & KMOD_CAPS)   strcat(s,"KMOD_CAPS ");
        if(n & KMOD_MODE)   strcat(s,"KMOD_MODE ");
        if(n & KMOD_SCROLL) strcat(s,"KMOD_SCROLL ");
    }
    SDL_Log("                e.keysym.mod      = (0x%x) %s", n, s);
    n = e->keysym.scancode;
    SDL_Log("                e.keysym.scancode = (0x%x) %s", n, SDL_GetScancodeName(n));
    n = e->keysym.sym;
    SDL_Log("                e.keysym.sym      = (0x%x) %s", n, SDL_GetKeyName(n));
}
void tmp_understand_mousem(SDL_Event *in, char *buf) {
    tmp_understand_plain(in,buf);
    SDL_MouseMotionEvent *e = (SDL_MouseMotionEvent *)in;
    SDL_Log("                SDL_MouseMotionEvent");
    SDL_Log("                e.windowID= %u", e->windowID);
    SDL_Log("                e.which   = %u", e->which);
    SDL_Log("                e.x       = %d", e->x);
    SDL_Log("                e.y       = %d", e->y);
    SDL_Log("                e.xrel    = %d", e->xrel);
    SDL_Log("                e.yrel    = %d", e->yrel);
}
void tmp_understand_mouseb(SDL_Event *in, char *buf) {
    tmp_understand_plain(in,buf);
    SDL_MouseButtonEvent *e = (SDL_MouseButtonEvent *)in;
    SDL_Log("                SDL_MouseMotionEvent");
    SDL_Log("                e.windowID= %u", e->windowID);
    SDL_Log("                e.which   = %u", e->which);
    SDL_Log("                e.button  = %d", e->button);
    SDL_Log("                e.state   = %d", e->state);
    SDL_Log("                e.clicks  = %d", e->clicks);
    SDL_Log("                e.x       = %d", e->x);
    SDL_Log("                e.y       = %d", e->y);
}
void tmp_understand_mousew(SDL_Event *in, char *buf) {
    tmp_understand_plain(in,buf);
    SDL_MouseWheelEvent *e = (SDL_MouseWheelEvent *)in;
    SDL_Log("                SDL_MouseMotionEvent");
    SDL_Log("                e.windowID= %u", e->windowID);
    SDL_Log("                e.which   = %u", e->which);
    SDL_Log("                e.x       = %d", e->x);
    SDL_Log("                e.y       = %d", e->y);
    SDL_Log("                e.direction = %d", e->direction);
    SDL_Log("                e.preciseX= %f", e->preciseX);
    SDL_Log("                e.preciseY= %f", e->preciseY);
    SDL_Log("                e.mouseX  = %d", e->mouseX);
    SDL_Log("                e.mouseY  = %d", e->mouseY);
}
void tmp_understand(SDL_Event *e) {
    char buf[1024];
    SDL_CommonEvent *ev = (SDL_CommonEvent *)e;
    switch(ev->type) {
        case SDL_QUIT: sprintf(buf, "SDL_QUIT"); tmp_understand_plain(e,buf); break;
        case SDL_APP_TERMINATING: sprintf(buf, "SDL_APP_TERMINATING"); tmp_understand_plain(e,buf);   break;
        case SDL_APP_LOWMEMORY: sprintf(buf, "SDL_APP_LOWMEMORY");  tmp_understand_plain(e,buf); break;
        case SDL_APP_WILLENTERBACKGROUND: sprintf(buf, "SDL_APP_WILLENTERBACKGROUND"); tmp_understand_plain(e,buf);  break;
        case SDL_APP_DIDENTERBACKGROUND: sprintf(buf, "SDL_APP_DIDENTERBACKGROUND"); tmp_understand_plain(e,buf);  break;
        case SDL_APP_WILLENTERFOREGROUND: sprintf(buf, "SDL_APP_WILLENTERFOREGROUND");tmp_understand_plain(e,buf);   break;
        case SDL_APP_DIDENTERFOREGROUND: sprintf(buf, "SDL_APP_DIDENTERFOREGROUND"); tmp_understand_plain(e,buf);  break;
        case SDL_LOCALECHANGED: sprintf(buf, "SDL_LOCALECHANGED"); tmp_understand_plain(e,buf); break;
        case SDL_DISPLAYEVENT: sprintf(buf, "SDL_DISPLAYEVENT");   tmp_understand_display(e,buf); break;
        case SDL_WINDOWEVENT: sprintf(buf, "SDL_WINDOWEVENT"); tmp_understand_window(e,buf);  break;
        case SDL_SYSWMEVENT: sprintf(buf, "SDL_SYSWMEVENT");  tmp_understand_plain(e,buf); break;
        case SDL_KEYDOWN: sprintf(buf, "SDL_KEYDOWN"); tmp_understand_keyboard(e,buf);  break;
        case SDL_KEYUP: sprintf(buf, "SDL_KEYUP"); tmp_understand_keyboard(e,buf); break;
        case SDL_TEXTEDITING: sprintf(buf, "SDL_TEXTEDITING"); tmp_understand_plain(e,buf);  break;
        case SDL_TEXTINPUT: sprintf(buf, "SDL_TEXTINPUT"); tmp_understand_plain(e,buf); break;
        case SDL_KEYMAPCHANGED: sprintf(buf, "SDL_KEYMAPCHANGED"); tmp_understand_plain(e,buf); break;
        case SDL_TEXTEDITING_EXT: sprintf(buf, "SDL_TEXTEDITING_EXT"); tmp_understand_plain(e,buf); break; //composition
        case SDL_MOUSEMOTION: sprintf(buf, "SDL_MOUSEMOTION"); tmp_understand_mousem(e,buf); break;
        case SDL_MOUSEBUTTONDOWN: sprintf(buf, "SDL_MOUSEBUTTONDOWN"); tmp_understand_mouseb(e,buf); break;
        case SDL_MOUSEBUTTONUP: sprintf(buf, "SDL_MOUSEBUTTONUP"); tmp_understand_mouseb(e,buf); break;
        case SDL_MOUSEWHEEL: sprintf(buf, "SDL_MOUSEWHEEL"); tmp_understand_mousew(e,buf); break;
        case SDL_FINGERDOWN: sprintf(buf, "SDL_FINGERDOWN"); tmp_understand_plain(e,buf); break;
        case SDL_FINGERUP: sprintf(buf, "SDL_FINGERUP"); tmp_understand_plain(e,buf); break;
        case SDL_FINGERMOTION: sprintf(buf, "SDL_FINGERMOTION"); tmp_understand_plain(e,buf); break;
        case SDL_DOLLARGESTURE: sprintf(buf, "SDL_DOLLARGESTURE"); tmp_understand_plain(e,buf); break;
        case SDL_DOLLARRECORD: sprintf(buf, "SDL_DOLLARRECORD"); tmp_understand_plain(e,buf); break;
        case SDL_MULTIGESTURE: sprintf(buf, "SDL_MULTIGESTURE"); tmp_understand_plain(e,buf); break;
        case SDL_CLIPBOARDUPDATE: sprintf(buf, "SDL_CLIPBOARDUPDATE"); tmp_understand_plain(e,buf); break;
        case SDL_DROPFILE: sprintf(buf, "SDL_DROPFILE"); tmp_understand_plain(e,buf); break;
        case SDL_DROPTEXT: sprintf(buf, "SDL_DROPTEXT"); tmp_understand_plain(e,buf); break;
        case SDL_DROPBEGIN: sprintf(buf, "SDL_DROPBEGIN"); tmp_understand_plain(e,buf); break;
        case SDL_DROPCOMPLETE: sprintf(buf, "SDL_DROPCOMPLETE"); tmp_understand_plain(e,buf); break;
        case SDL_AUDIODEVICEADDED: sprintf(buf, "SDL_AUDIODEVICEADDED"); tmp_understand_plain(e,buf); break;
        case SDL_AUDIODEVICEREMOVED: sprintf(buf, "SDL_AUDIODEVICEREMOVED"); tmp_understand_plain(e,buf); break;
        case SDL_SENSORUPDATE: sprintf(buf, "SDL_SENSORUPDATE"); tmp_understand_plain(e,buf); break;
        case SDL_RENDER_TARGETS_RESET: sprintf(buf, "SDL_RENDER_TARGETS_RESET"); tmp_understand_plain(e,buf); break;
        case SDL_RENDER_DEVICE_RESET: sprintf(buf, "SDL_RENDER_DEVICE_RESET"); tmp_understand_plain(e,buf); break;
        case SDL_POLLSENTINEL: sprintf(buf, "SDL_POLLSENTINEL"); tmp_understand_plain(e,buf); break;
        default:
            sprintf(buf,"UNKNOWN/UNHANDLED EVENT %08x",ev->type);
            tmp_understand_plain(e,buf);
            break;
    }
}
void sdl_loop() {
    sdl_init(1024,1024);
    int run = 1;
    while (run) {
        if(window != NULL) {
            SDL_Event e;

            if(SDL_PollEvent(&e)) {
                tmp_understand(&e);
                int temp =0;
                switch (e.type) {
                    case SDL_QUIT:
                        run = 0;
                        break;
                    case SDL_MOUSEMOTION:
                    case SDL_MOUSEBUTTONUP:
                    case SDL_MOUSEBUTTONDOWN:
                        temp = SDL_GetMouseState(NULL, NULL); // probably not the right way to do things
                        post_mouse(e.button.x, e.button.y,temp, e.common.timestamp);
                    default:
                        break;
                }
            }
        }
    }
    SDL_Log("Bye!");
    exit(-1);
}

