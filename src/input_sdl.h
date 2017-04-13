#ifndef INPUT_SDL_H
#define INPUT_SDL_H

#include "bus.h"

class SDLInputDevice : public IInputDevice
{
public:
    SDLInputDevice(IBus &bus, IController &controller);
    virtual ~SDLInputDevice() {}

public:
    virtual void tick() override;

private:
    IBus &_bus;
    IController &_port;
};

#endif
