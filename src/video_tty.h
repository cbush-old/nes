#ifndef VIDEO_TTY_H
#define VIDEO_TTY_H

#include "bus.h"

class TTYVideoDevice : public IVideoDevice {
  public:
    ~TTYVideoDevice(){}

  public:
    void on_frame() override;
    void put_pixel(uint8_t x, uint8_t y, PaletteIndex i) override;

};

#endif
