//
// Created by Simon Waite on 18/03/2024.
//

#include "sdlstub.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

SDL_Window volatile* window = NULL;
SDL_Renderer volatile* renderer = NULL;


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
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void*)rgb,w,h,24, p,0xFF0000, 0x00FF00, 0x0000FF, 0 );
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


}

void sdl_loop() {
    sdl_init(1024,1024);

    while (1) {
        if(window != NULL) {
            SDL_Event e;

            if(SDL_PollEvent(&e)) {
                switch (e.type) {
                    case SDL_QUIT:
                        exit(-1);
                }
            }
        }
    }
}

