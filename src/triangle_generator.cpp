#include "triangle_generator.h"

static const uint8_t TRIANGLE_STEPS[]
{
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
};

Triangle::~Triangle() {}

void Triangle::on_quarter_frame()
{
    if (_linear_counter_reload)
    {
        _linear_counter = _linear_counter_reload_value;
    }
    else if (_linear_counter > 0)
    {
        --_linear_counter;
    }

    if (_triangle_control)
    {
        _linear_counter_reload = false;
    }
}

bool Triangle::get_length_counter_halt() const
{
    return _triangle_length_counter_halt;
}

void Triangle::reg3_write(uint8_t value)
{
    reg3 = value;
    _triangle_length_counter_halt = true;
    reload_length_counter();
    _linear_counter_reload = true;
}

void Triangle::update()
{
    if (++t > _timer)
    {
        t = 0;
        if (length_counter_active() && _linear_counter > 0)
        {
            step = (step + 1) & 31;
        }
    }
    
    _sample = (!_enabled || !length_counter_active() || _timer < 2) ? 0 : TRIANGLE_STEPS[step];
}
