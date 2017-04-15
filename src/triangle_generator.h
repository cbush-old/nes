#pragma once

#include "generator.h"

struct Triangle : Generator_with_length_counter
{
public:
    virtual ~Triangle();

    void on_quarter_frame();
    void update();

private:
    virtual bool get_length_counter_halt() const override;

    virtual void on_reg3_write(uint8_t value) override;

    uint16_t t{ 0 };
    uint8_t step{ 0 };
    bool _linear_counter_reload{ false };
    uint8_t _linear_counter{ 0 };
};
