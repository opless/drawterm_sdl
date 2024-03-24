//
// Created by Simon Waite on 18/03/2024.
//


void sdl_get_screen(int *x, int *y);
void sdl_loop();

void sdl_cursor_show(int flag, unsigned char *data,unsigned char *mask, int width, int height, int x, int y);
void sdl_cursor_move(int x, int y);

void sdl_update(unsigned char * argb32);