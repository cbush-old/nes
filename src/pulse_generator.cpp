#include "pulse_generator.h"

static const uint8_t PULSE_WAVES[]
{
    0x40, // 0 1 0 0 0 0 0 0 (12.5%)
    0x60, // 0 1 1 0 0 0 0 0 (25%)
    0x78, // 0 1 1 1 1 0 0 0 (50%)
    0x9f, // 1 0 0 1 1 1 1 1 (25% negated)
};

void Pulse::on_reg3_write(uint8_t value)
{
    reload_length_counter();
    shift = 0; // restart sequencer
    _silenced = false;
    start_envelope();
}

void Pulse::on_reg1_write(uint8_t value)
{
    adjust_period();
}

void Pulse::adjust_period()
{
    _target = _timer + (_sweep_negative ? -1 : 1) * (_timer >> _sweep_shift);
    if (_timer < 8 || _target > 0x7ff)
    {
        _silenced = true;
    }
}

void Pulse::on_half_frame()
{
    clock_length_counter();
    if (_sweep_reload)
    {

        if (!_sweep_divider && _sweep_enabled)
        {
            adjust_period();
        }

        _sweep_divider = _sweep_period + 1;
        _sweep_reload = false;
    }
    else
    {

        if (!_sweep_divider)
        {
            if (_sweep_enabled)
            {
                _sweep_divider = _sweep_period + 1;
                adjust_period();
                if (!_silenced && _sweep_shift)
                {
                    set_timer(_target);
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
    if (++t > _timer)
    {
        t = 0;
        ++shift;
    }
    _sample = (_silenced || !length_counter_active()) ? 0 : bool(PULSE_WAVES[_duty] & (1 << (shift & 7))) * envelope_sample();
}
