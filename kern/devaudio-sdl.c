/*
 * SDL
 */
#include <sys/ioctl.h>

#include	"u.h"
#include	"lib.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"
#include	"devaudio.h"

#include "../gui-sdl/sdl_audio.h"




/* maybe this should return -1 instead of sysfatal */
void
audiodevopen(void)
{
    if(sdl_audio_open())
        error("cannot open");
}

void
audiodevclose(void)
{
    if(sdl_audio_close())
        error("cannot close");
}

void
audiodevsetvol(int what, int left, int right)
{
    error("no volume support");
}

void
audiodevgetvol(int what, int *left, int *right)
{
    error("no volume support");
}

int
audiodevwrite(void *v, int n)
{
    int rc = sdl_audio_write(v,n);
    if(rc <0)
        error("no write");
    return rc;
}

int
audiodevread(void *v, int n)
{
    if(sdl_audio_read(v,n) <0)
        error("no read");
	return -1;
}
