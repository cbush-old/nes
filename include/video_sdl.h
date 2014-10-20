#ifndef VIDEO_SDL_H
#define VIDEO_SDL_H

#include "bus.h"
#include "observable.h"

class SDLVideoDevice : public IVideoDevice, public IObserver<uint8_t>, public IObserver<uint16_t> {
  public:
    SDLVideoDevice();
    ~SDLVideoDevice();

  public:
    void set_buffer(Framebuffer const& buffer) override;
    void on_frame() override;

  public:
    void on_change(observable<uint16_t> const*, uint16_t, uint16_t) override;
    void on_change(observable<uint8_t> const*, uint8_t, uint8_t) override;

  private:
    struct SDL_Window *window;
    void *glcon;
    uint32_t texture;
    Framebuffer const *_buffer;
    Framebuffer _buffer2;

};

#endif