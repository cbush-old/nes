#pragma once

#include "generator.h"

struct Noise : Generator_with_envelope
{
public:
    virtual void reg3_write(uint8_t value) override;
    
    void update();

private:
    int t{ 0 };
    uint16_t shift{ 1 };
};
