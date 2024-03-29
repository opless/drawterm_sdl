#include "u.h"
#include "lib.h"
#include "dat.h"
#include "fns.h"
#include "error.h"

#include <draw.h>
#include <memdraw.h>

#include "screen.h"

#include "sdl.h"

Memimage *gscreen = NULL;
QLock snarf_lock;

void screeninit() {
    ulong chan = ARGB32;
    Rectangle r = Rect(0,0,0,0);
    memimageinit();
    sdl_get_size(&r.max.x,&r.max.y);
    screensize(r,chan);
    if(!gscreen) {
        panic("screeninit: screensize failed.");
    }
    gscreen->clipr = r;

    qlock(&drawlock);
    terminit();
    flushmemscreen(gscreen->clipr);
    qunlock(&drawlock);
}

void screensize(Rectangle r, ulong chan) {
    if(gscreen) {
        sdl_update(NULL); // invalidate sdl's idea of the screen
        freememimage(gscreen);
        gscreen = NULL;
    }

    Memimage *mi = allocmemimage(r,chan);
    if(! mi) {
        return;
    }
    gscreen = mi;
    gscreen->clipr = ZR;
}

Memdata *attachscreen(Rectangle *r, ulong *chan, int *depth, int *width, int *softscreen) {
    *r = gscreen->clipr;
    *chan = gscreen->chan;
    *depth= gscreen->depth;
    *width=gscreen->width;
    *softscreen = 1;
    gscreen->data->ref++;
    return gscreen->data;
}

char *clipread() {
    qlock(&snarf_lock);
    char *t = sdl_clipboard_read();
    qunlock(&snarf_lock);
    return t;
}

int clipwrite(char *buf) {
    qlock(&snarf_lock);
    int r = sdl_clipboard_write(buf);
    qunlock(&snarf_lock);
    return r; // -1 is an error
}

void flushmemscreen(Rectangle r) {
    // check for overlap or zero lengths
    if(rectclip(&r, gscreen->clipr) == 0 || Dx(r) == 0 || Dy(r) == 0)
        return;
    sdl_update(gscreen->data->bdata);
}

void setcolor(ulong i, ulong r, ulong g, ulong b) {
    USED(i);
    USED(r);
    USED(g);
    USED(b);
}

void getcolor(ulong i, ulong *r, ulong *g, ulong *b) {
    ulong v = cmap2rgb(i);
    *r = (v >> 16) & 0xFF;
    *g = (v >> 8) & 0xFF;
    *b = v & 0xFF;
}

void  mouseset(Point p) {
    setcursor();
    sdl_cursor_move(p.x,p.y);
}

void setcursor() {
    uchar src[32], mask[32];

    for (int i = 0; i < 32; i++) {
        src[i] = cursor.set[i];
        mask[i] = cursor.set[i] | cursor.clr[i];
    }
    sdl_cursor_set(src, mask, -cursor.offset.x, -cursor.offset.y);

    qlock(&drawlock);
    flushmemscreen(gscreen->clipr);
    qunlock(&drawlock);
}

static void cpuproc(void *arg) {
    cpubody();
}

void start_cpu() {
    kproc("cpu", cpuproc, nil);
}

void guimain(void)
{
    sdl_loop();
}

void post_mouse(int x, int y, int b, unsigned long t) {
    absmousetrack(x, y, b, t);
}

void post_keyboard(int rune,int down) {
    kbdkey((Rune)rune,down);
}

void post_resize(int w, int h) {
    screenresize(Rect(0,0,w,h));
}