#ifndef VIDEO_TTY_H
#define VIDEO_TTY_H

#include "bus.h"

class TTYVideoDevice : public IVideoDevice {
  public:
    ~TTYVideoDevice(){}

  public:
    virtual void set_buffer(Framebuffer const& buffer) override;
    virtual void on_frame() override;

};

#endif
