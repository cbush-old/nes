#pragma once

#include "generator.h"

struct Noise : Generator_with_envelope
{
    virtual ~Noise();
    virtual void reg3_write(uint8_t value) override;
    virtual void update() override;
    virtual double sample() const override;

    int t{ 0 };
    uint16_t shift{ 1 };
};
