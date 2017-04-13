#ifndef INPUT_RNG_H
#define INPUT_RNG_H

#include "bus.h"

#include <iostream>
#include <random>

class RNGInputDevice : public IInputDevice
{
public:
    RNGInputDevice(IController &controller);
    ~RNGInputDevice() {}

public:
    void tick();

private:
    IController &port;
    std::mt19937 gen;
};

#endif
