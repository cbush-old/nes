#include "input_rng.h"

#include <ctime>
#include <iostream>

RNGInputDevice::RNGInputDevice(IController &controller)
    : port(controller)
{
    gen.seed(time(0));
}

void RNGInputDevice::tick()
{
    IController::Button n = (IController::Button)(gen() % IController::N_BUTTONS);
    int on = gen() & 2;
    port.set_button_state(n, on);
}
