//
// Created by Simon Waite on 18/03/2024.
//


int sdl_write(unsigned char* rgb,int xmin, int ymin,int xmax,int ymax);
void sdl_get_screen(int *x, int *y);
void sdl_loop();

void sdl_cursor_show(int flag);
void sdl_cursor_move(int x, int y);