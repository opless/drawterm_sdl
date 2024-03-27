
#include "sdl_audio.h"

#include <SDL.h>

static SDL_AudioDeviceID sdl_audio_out_dev = 0;

int sdl_audio_open() {
    SDL_Log("Using audio driver: %s", SDL_GetCurrentAudioDriver());
    if(SDL_GetCurrentAudioDriver() == NULL) {
        return -1;
    }
    return 0;
}
int sdl_audio_close() {
    SDL_Log("(not) closing audio driver: %s\n", SDL_GetCurrentAudioDriver());
    return 0;
}

int sdl_audio_init() {
    //SDL_Log("%s %s:%d",__PRETTY_FUNCTION__ , __FILE__ , __LINE__);
    if(SDL_GetCurrentAudioDriver() == NULL) {
        return -1;
    }
    if(!sdl_audio_out_dev) {
        SDL_AudioSpec want, have;
        SDL_AudioDeviceID dev;

        SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
        want.freq = 44100;
        want.format = AUDIO_S16LSB;
        want.channels = 2;
        want.samples = 512; //4096;
        want.callback = NULL;
        want.userdata = NULL;
        dev = SDL_OpenAudioDevice(NULL,
                                  0,
                                  &want,
                                  &have,
                                  SDL_AUDIO_ALLOW_FORMAT_CHANGE);
        SDL_PauseAudioDevice(dev, 0);
        if(dev) {
            sdl_audio_out_dev = dev;
        } else {
            SDL_Log("SDL_OpenAudioDevice (output) Error: %s", SDL_GetError());
        }
    }
    return (sdl_audio_out_dev == 0 ? -1 : 0);
}
int sdl_audio_read(void *p, int n) {

    return -1;

}
int sdl_audio_write(void *p, int n) {
    if(sdl_audio_init()) {
        return -1;
    }
    int waiting = SDL_GetQueuedAudioSize(sdl_audio_out_dev);
    // 44100 samples per second.
    // 1 second = 44100 x 2 x 2 = 176400 samples
    // 176400 x 2 x 2 = 705,600 bytes/sec
    while(waiting > 32768) {
        SDL_Delay(10);
        waiting = SDL_GetQueuedAudioSize(sdl_audio_out_dev);
    }
    int rc = SDL_QueueAudio(sdl_audio_out_dev, p, n);
    if(rc) {
        SDL_Log("SDL_QueueAudio (output) Error: %s", SDL_GetError());
        return -1;
    }
    return n;
}