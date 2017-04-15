#include "generator.h"

static const uint8_t LENGTHS[]
{
    //          0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
    /* 0x0_ */ 10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
    /* 0x1_ */ 12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};

Generator::~Generator() = default;

void Generator::regw(size_t r, uint8_t value)
{
    switch (r & 0x3)
    {
        case 0: reg0 = value; break;
        case 1: reg1_write(value); break;
        case 2: reg2 = value; break;
        case 3: reg3_write(value); break;
    }
}

void Generator::set_channel_volume(int16_t v)
{
    _channel_volume = v;
}

int16_t Generator::get_channel_volume() const
{
    return _channel_volume;
}

void Generator::reg1_write(uint8_t value)
{
    reg1 = value;
}


int16_t Generator::mixed_sample() const
{
    return (static_cast<int16_t>(_sample) - 0x80) * _channel_volume;
}

void Generator::enable()
{
    _enabled = true;
}

void Generator::disable()
{
    _enabled = false;
}

Generator_with_length_counter::~Generator_with_length_counter() = default;

void Generator_with_length_counter::on_half_frame()
{
    clock_length_counter();
}

void Generator_with_length_counter::reload_length_counter()
{
    if (_enabled)
    {
        _counter = LENGTHS[length_counter_load];
    }
}

void Generator_with_length_counter::disable()
{
    _enabled = false;
    _counter = 0;
}

bool Generator_with_length_counter::get_length_counter_halt() const
{
    return length_counter_halt;
}

bool Generator_with_length_counter::length_counter_active() const
{
    return _counter > 0;
}

void Generator_with_length_counter::clock_length_counter()
{
    if (!get_length_counter_halt() && _counter > 0)
    {
        --_counter;
    }
}

void Generator_with_envelope::divider_reload()
{
    _divider_counter = divider_period;
}

void Generator_with_envelope::divider_clock()
{
    if (_divider_counter == 0)
    {
        divider_reload();
        on_divider_output_clock();
    }
    else
    {
        --_divider_counter;
    }
}

void Generator_with_envelope::on_divider_output_clock()
{
    if (_counter == 0)
    {
        if (envelope_loop)
        {
            _counter = 15;
        }
    }
    else
    {
        --_counter;
    }
}

uint8_t Generator_with_envelope::envelope_sample() const
{
    return constant_volume ? volume : _counter;
}

void Generator_with_envelope::on_quarter_frame()
{
    if (!_started)
    {
        divider_clock();
    }
    else
    {
        _started = false;
        _counter = 15;
        divider_reload();
    }
}

void Generator_with_envelope::start_envelope()
{
    _started = true;
}

Generator_with_envelope::~Generator_with_envelope() = default;
