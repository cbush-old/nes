#pragma once

#include "generator.h"

struct Triangle : Generator_with_length_counter
{
    virtual ~Triangle();

    virtual void on_quarter_frame() override;

    virtual bool get_length_counter_halt() const override;

    virtual void reg3_write(uint8_t value) override;

    virtual void update() override;

    virtual double sample() const override;

    uint16_t t{ 0 };
    uint8_t step{ 0 };
    bool _linear_counter_reload{ false };
    uint8_t _linear_counter{ 0 };
};
