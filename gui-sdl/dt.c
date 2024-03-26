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
char *snarf_buff  = NULL;

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
    if(snarf_buff) {
        return strdup(snarf_buff);
    }
    return nil;
}

int clipwrite(char *buf) {
    if(snarf_buff) {
        free(snarf_buff);
    }
    snarf_buff = strdup(buf);
    return snarf_buff ? 0 : -1; // -1 is an error
}

void flushmemscreen(Rectangle r) {
    // We don't actually use r, as the buffer is already written to
    USED(r);
    // check for overlap
    if(rectclip(&r, gscreen->clipr) == 0) {
        return;
    }

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