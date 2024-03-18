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

Memimage *gscreen;
char     *snarfbuf;

int count=1000;

//screenr is size of entils re screen
Rectangle	screenr;

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

/*! \def SVPNG_OUTPUT
    \brief User customizable output stream.
    By default, it uses C file descriptor and fputc() to output bytes.
    In C++, for example, user may use std::ostream or std::vector instead.
*/
#ifndef SVPNG_OUTPUT
#include <stdio.h>
#define SVPNG_OUTPUT int fp
#endif

/*! \def SVPNG_PUT
    \brief Write a byte
*/
#ifndef SVPNG_PUT
#define SVPNG_PUT(u) { unsigned char x[1];x[0] = u; write(fp, x,  1); }
#endif


/*!
    \brief Save a RGB/RGBA image in PNG format.
    \param SVPNG_OUTPUT Output stream (by default using file descriptor).
    \param w Width of the image. (<16383)
    \param h Height of the image.
    \param img Image pixel data in 24-bit RGB or 32-bit RGBA format.
    \param alpha Whether the image contains alpha channel.
*/
SVPNG_LINKAGE void svpng(SVPNG_OUTPUT, unsigned w, unsigned h, const unsigned char* img, int alpha) {
    static const unsigned t[] = { 0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
            /* CRC32 Table */    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
    unsigned a = 1, b = 0, c, p = w * (alpha ? 4 : 3) + 1, x, y, i;   /* ADLER-a, ADLER-b, CRC, pitch */
#define SVPNG_U8A(ua, l) for (i = 0; i < l; i++) SVPNG_PUT((ua)[i]);
#define SVPNG_U32(u) do { SVPNG_PUT((u) >> 24); SVPNG_PUT(((u) >> 16) & 255); SVPNG_PUT(((u) >> 8) & 255); SVPNG_PUT((u) & 255); } while(0)
#define SVPNG_U8C(u) do { SVPNG_PUT(u); c ^= (u); c = (c >> 4) ^ t[c & 15]; c = (c >> 4) ^ t[c & 15]; } while(0)
#define SVPNG_U8AC(ua, l) for (i = 0; i < l; i++) SVPNG_U8C((ua)[i])
#define SVPNG_U16LC(u) do { SVPNG_U8C((u) & 255); SVPNG_U8C(((u) >> 8) & 255); } while(0)
#define SVPNG_U32C(u) do { SVPNG_U8C((u) >> 24); SVPNG_U8C(((u) >> 16) & 255); SVPNG_U8C(((u) >> 8) & 255); SVPNG_U8C((u) & 255); } while(0)
#define SVPNG_U8ADLER(u) do { SVPNG_U8C(u); a = (a + (u)) % 65521; b = (b + a) % 65521; } while(0)
#define SVPNG_BEGIN(s, l) do { SVPNG_U32(l); c = ~0U; SVPNG_U8AC(s, 4); } while(0)
#define SVPNG_END() SVPNG_U32(~c)
    SVPNG_U8A("\x89PNG\r\n\32\n", 8);           /* Magic */
    SVPNG_BEGIN("IHDR", 13);                    /* IHDR chunk { */
    SVPNG_U32C(w); SVPNG_U32C(h);               /*   Width & Height (8 bytes) */
    SVPNG_U8C(8); SVPNG_U8C(alpha ? 6 : 2);     /*   Depth=8, Color=True color with/without alpha (2 bytes) */
    SVPNG_U8AC("\0\0\0", 3);                    /*   Compression=Deflate, Filter=No, Interlace=No (3 bytes) */
    SVPNG_END();                                /* } */
    SVPNG_BEGIN("IDAT", 2 + h * (5 + p) + 4);   /* IDAT chunk { */
    SVPNG_U8AC("\x78\1", 2);                    /*   Deflate block begin (2 bytes) */
    for (y = 0; y < h; y++) {                   /*   Each horizontal line makes a block for simplicity */
        SVPNG_U8C(y == h - 1);                  /*   1 for the last block, 0 for others (1 byte) */
        SVPNG_U16LC(p); SVPNG_U16LC(~p);        /*   Size of block in little endian and its 1's complement (4 bytes) */
        SVPNG_U8ADLER(0);                       /*   No filter prefix (1 byte) */
        for (x = 0; x < p - 1; x++, img++)
            SVPNG_U8ADLER(*img);                /*   Image pixel data */
    }
    SVPNG_U32C((b << 16) | a);                  /*   Deflate block end with adler (4 bytes) */
    SVPNG_END();                                /* } */
    SVPNG_BEGIN("IEND", 0); SVPNG_END();        /* IEND chunk {} */
}



void write_png_file(const char *filename, int width, int height, unsigned char *rgb_data) {
    int fp = open(filename,O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fp <0) {
        XLOG("Error: Unable to open file %s for writing.", filename);
        return;
    }

    svpng(fp, 1024, 1024, gscreen->data->bdata, 0);

    close(fp);
}
void screeninit(void) {
    ulong chan;
    XLOG("screeninit");
    memimageinit();
    snarfbuf = NULL;
    chan = RGB24;
    screenr.min.x = 0;
    screenr.min.y = 0;
    screenr.max.x = 1024;
    screenr.max.y = 1024;
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
    XLOG("  chan = %lu", chan);
    Memimage *i;

    if((i = allocmemimage(r, chan)) == nil)
        return;
    if(gscreen != nil)
        freememimage(gscreen);
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
    XLOG("flushmemscreen %d-%d, %d-%d",r.min.x,r.max.x,r.min.y,r.max.y);

    // copy area of gscreen to actual display

    char name[2048];
    sprintf(name,"screen-%08d_%d-%d_%d-%d.png",count++,r.min.x,r.max.x,r.min.y,r.max.y);
    write_png_file(name,1024,1024,gscreen->data->bdata);

    // just say the mouse is on the screen somewhere
    ulong msec = ticks();
    absmousetrack(256, 256, 0, msec);

}


void mouseset(Point p) {
    XLOG("mouseset %d,%d",p.x,p.y);

    // draw mouse on screen, or warp hardware cursor to position
    /*
     * qlock(&drawlock);
     * mousexy = p;
     * flushmemscreen(screenr);
     * qunlock(&drawlock);
     */
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

    qlock(&drawlock);
    flushmemscreen(screenr);
    qunlock(&drawlock);

}

void guimain(void)
{
    XLOG("guimain");

    cpubody();
}