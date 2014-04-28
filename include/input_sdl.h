#ifndef INPUT_SDL_H
#define INPUT_SDL_H

#include "bus.h"

class SDLInputDevice : public IInputDevice {
  public:
    SDLInputDevice(IController& controller);
    ~SDLInputDevice(){}

  public:
    void tick();

    int get_frameskip() const;

  private:
    IController& port;
};

#endif