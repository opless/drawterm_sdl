//
// Created by Simon Waite on 17/03/2024.
//
#include "u.h"
#include "lib.h"
#include "dat.h"
#include "fns.h"
#include "error.h"

#include <draw.h>
#include <memdraw.h>
//#include <keyboard.h>
//#include <cursor.h>
#include "screen.h"

#include "sdlstub.h"

Memimage *gscreen;
char     *snarfbuf;

int count=1000;

//screenr is size of entils re screen

#define XLOG(...) { \
    char s[2048]; \
    sprintf(s,__VA_ARGS__); \
    write(2,s,strlen(s));   \
    write(2,"\n",1); \
    }


/*! \file
    \brief      svpng() is a minimalistic C function for saving RGB/RGBA image into uncompressed PNG.
    \author     Milo Yip
    \version    0.1.1
    \copyright  MIT license
    \sa         http://github.com/miloyip/svpng
*/

/*! \def SVPNG_LINKAGE
    \brief User customizable linkage for svpng() function.
    By default this macro is empty.
    User may define this macro as static for static linkage,
    and/or inline in C99/C++, etc.
*/
#ifndef SVPNG_LINKAGE
#define SVPNG_LINKAGE
#endif


void screeninit(void) {
    ulong chan;
    XLOG("screeninit");
    memimageinit();
    snarfbuf = NULL;
    chan = ARGB32;  // is there a preferred pixel format with sdl? rgb24 likes only even sizes
    // mobiles prefer abgr, opengl/d3d argb
    Rectangle screenr;
    screenr.min.x = 0;
    screenr.min.y = 0;
    sdl_get_screen(&screenr.max.x,&screenr.max.y);

    screensize(screenr, chan);
    if(gscreen == nil) {
        panic("screensize failed");
    }
    gscreen->clipr = screenr;

    qlock(&drawlock);
    terminit();
    flushmemscreen(gscreen->clipr);
    qunlock(&drawlock);
}


Memdata*
attachscreen(Rectangle *r, ulong *chan, int *depth, int *width, int *softscreen) {
    XLOG("attachscreen");

    *r = gscreen->clipr;
    *chan = gscreen->chan;
    *depth = gscreen->depth;
    *width = gscreen->width;
    *softscreen = 1;

    gscreen->data->ref++;
    return gscreen->data;
}

void screensize(Rectangle r, ulong chan)
{
    XLOG("screensize");
    XLOG("  r= %d-%d, %d-%d",r.min.x,r.max.x,r.min.y,r.max.y);
    XLOG("  chan = %u", chan);
    Memimage *i;

    if((i = allocmemimage(r, chan)) == nil) {
        XLOG("oops");
        return;
    }
    if(gscreen != nil) {
        XLOG("err");
        freememimage(gscreen);
    }
    gscreen = i;
    gscreen->clipr = ZR;
}

char *clipread() {
    XLOG("clipread");

    if(snarfbuf)
        return strdup(snarfbuf);
    return nil;
}

int clipwrite(char *buf) {
    XLOG("clipwrite %p", buf);

    if(snarfbuf)
        free(snarfbuf);
    snarfbuf = strdup(buf);
    return 0;
    // -1 error
}

void flushmemscreen(Rectangle r) {
    XLOG("flushmemscreen %d-%d, %d-%d", r.min.x, r.max.x, r.min.y, r.max.y);

    // if no overlap then don't bother
    if (rectclip(&r, gscreen->clipr) == 0) {
        XLOG("... no overlap");
        return;
    }

    // if anything turns out negative, or zero ... whinge, and bail.
    int width = r.max.x - r.min.x;
    int height = r.max.y - r.min.y;

    if(width <= gscreen->clipr.min.x || width > gscreen->clipr.max.x ||
            height <= gscreen->clipr.min.y || height > gscreen->clipr.max.y
            ) {
        XLOG("Width = %d, Height = %d which is out of bounds, bailing ",width, height);
        return;
    }

    int depth = gscreen->depth;
    int pitch = gscreen->nchan * width;

    Memimage *changed = NULL;

    Rectangle rect = Rect(0,0,width,height);
    // copy into small rectangle and send it for writing to screen.
    changed = allocmemimage(r,gscreen->chan);
    if(changed) {
        memimagedraw(changed, rect, gscreen, r.min, nil, r.min, S);
        sdl_write(changed->data->bdata,
                  depth, pitch,
                  r.min.x,r.min.y,
                  r.max.x,r.max.y);
        freememimage(changed);
    } else {
        // if full screen, or can't allocate small memory
        sdl_write(gscreen->data->bdata,
                  depth,pitch,
                  gscreen->clipr.min.x, gscreen->clipr.min.y,
                  gscreen->clipr.max.x, gscreen->clipr.max.y);
    }
}


void mouseset(Point p) {
    XLOG("mouseset %d,%d",p.x,p.y);
    setcursor();
    sdl_cursor_move(p.x,p.y);
}


void setcolor(ulong i, ulong r, ulong g, ulong b) {
    // no-op
    XLOG("setcolor");

}
void getcolor(ulong i, ulong *r, ulong *g, ulong *b) {
    XLOG("getcolor %ul",i);

    ulong v;

    v = cmap2rgb(i);
    *r = (v >> 16) & 0xFF;
    *g = (v >> 8) & 0xFF;
    *b = v & 0xFF;

}

void setcursor() {
    XLOG("setcursor");

    uchar src[2 * 16], mask[2 * 16];

    for (int i = 0; i < 2 * 16; i++) {
        src[i] = cursor.set[i];
        mask[i] = cursor.set[i] | cursor.clr[i];
    }

    sdl_cursor_show(1, src, mask, 16, 16, -cursor.offset.x, -cursor.offset.y);

    qlock(&drawlock);
    flushmemscreen(gscreen->clipr);
    qunlock(&drawlock);

}
static void cpuproc(void *arg) {
    XLOG("cpuproc");

    cpubody();
}
void start_cpu() {
    XLOG("start_cpu");

    kproc("cpu", cpuproc, nil);
}
void guimain(void)
{
    XLOG("guimain");

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
