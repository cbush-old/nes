#ifndef INPUT_SCRIPT_RECORDER_H
#define INPUT_SCRIPT_RECORDER_H

#include "bus.h"

#include <iostream>
#include <fstream>
#include <set>

class ScriptRecorder : public IInputDevice
{
public:
    ScriptRecorder(IController &controller);
    ~ScriptRecorder();

public:
    void tick();

private:
    IController &port;
    std::ofstream out{ "out.tas" };
    std::set<IController::Button> pressed;
    unsigned wait{ 0 };
};

#endif

f
