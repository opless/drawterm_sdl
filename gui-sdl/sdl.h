void sdl_loop();
void sdl_get_size(int *x, int *y);
void sdl_cursor_set(unsigned char *data,unsigned char *mask, int x, int y);
void sdl_cursor_move(int x, int y);
void sdl_update(unsigned char * argb32);
int sdl_clipboard_write(const char *buf);
char *sdl_clipboard_read();