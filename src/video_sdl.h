#ifndef VIDEO_SDL_H
#define VIDEO_SDL_H

#include "bus.h"
#include "observable.h"

#include "SDL.h"

class SDLVideoDevice
    : public IVideoDevice
{
public:
    SDLVideoDevice();
    ~SDLVideoDevice();

    virtual void on_frame() override;
    virtual void put_pixel(uint8_t x, uint8_t y, PaletteIndex i) override;

private:
    void screenshot();

    struct SDL_Window *_window;
    SDL_GLContext _gl_context;
    uint32_t _texture;
    Framebuffer _buffer;
};

#endif
