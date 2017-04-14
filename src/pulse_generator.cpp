#include "pulse_generator.h"

static const uint8_t PULSE_WAVES[]
{
    0x40, // 0 1 0 0 0 0 0 0 (12.5%)
    0x60, // 0 1 1 0 0 0 0 0 (25%)
    0x78, // 0 1 1 1 1 0 0 0 (50%)
    0x9f, // 1 0 0 1 1 1 1 1 (25% negated)
};

Pulse::~Pulse() {}

void Pulse::reg3_write(uint8_t value)
{
    reg3 = value;
    reload_length_counter();
    shift = 0; // restart sequencer
    _silenced = false;
    start_envelope();
}

void Pulse::reg1_write(uint8_t value)
{
    reg1 = value;
    adjust_period();
}

void Pulse::adjust_period()
{
    _target = timer + (sweep_negative ? -1 : 1) * (timer >> sweep_shift);
    if (timer < 8 || _target > 0x7ff)
    {
        _silenced = true;
    }
}

void Pulse::on_half_frame()
{
    clock_length_counter();
    if (_sweep_reload)
    {

        if (!_sweep_divider && sweep_enabled)
        {
            adjust_period();
        }

        _sweep_divider = sweep_period + 1;
        _sweep_reload = false;
    }
    else
    {

        if (!_sweep_divider)
        {
            if (sweep_enabled)
            {
                _sweep_divider = sweep_period + 1;
                adjust_period();
                if (!_silenced && sweep_shift)
                {
                    timer = _target;
                }
            }
        }
        else
        {

            --_sweep_divider;
        }
    }
}

void Pulse::update()
{
    if (++t > timer)
    {
        t = 0;
        ++shift;
    }
}

double Pulse::sample() const
{
    return 0.0;
    if (_silenced || !length_counter_active())
    {
        return 0.0;
    }

    return bool(PULSE_WAVES[duty] & (1 << (shift % 8))) * envelope_sample();
    
}
