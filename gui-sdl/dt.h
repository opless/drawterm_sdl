void start_cpu();
void post_mouse(int x, int y, int b, unsigned long t);
void post_mousewheel(int flipped, int dx,int dy, unsigned long t);
void post_keyboard(int rune,int down);
void post_resize(int w, int h);