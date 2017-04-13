#ifndef INPUT_SCRIPT_H
#define INPUT_SCRIPT_H

#include "bus.h"

#include <iostream>

class ScriptInputDevice : public IInputDevice
{
public:
    ScriptInputDevice(IController &controller, std::istream &source);
    ~ScriptInputDevice() {}

public:
    void tick();

private:
    IController &port;
    std::istream &source;
    unsigned wait{ 0 };
};

#endif
