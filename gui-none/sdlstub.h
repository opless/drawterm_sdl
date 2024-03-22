//
// Created by Simon Waite on 18/03/2024.
//


int sdl_write(int x, int y, unsigned char* rgb,int xmin, int xmax,int ymin,int ymax);

void sdl_loop();

void sdl_cursor_show(int flag);
void sdl_cursor_move(int x, int y);