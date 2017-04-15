#include "noise_generator.h"

static const uint16_t NOISE_PERIODS[]
{
    4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068,
};

void Noise::reg3_write(uint8_t value)
{
    reg3 = value;
    reload_length_counter();
    start_envelope();
}

// 7 6 5 4 3 2 1 0
//               ^- bit 14 set to bit 0 XOR...
//   ^         ^--- - bit 1 if !mode
//   +------------- - bit 6 if mode
void Noise::update()
{
    if (++t > NOISE_PERIODS[_noise_period])
    {
        t = 0;
        bool feedback = bool(shift & 1) ^ bool(shift & (2 << (_noise_mode * 5)));
        shift >>= 1;
        shift &= 0x1fff;
        shift |= feedback << 13;
    }
    
    if (!length_counter_active() || !_enabled)
    {
        _sample = 0;
    }
    else
    {
        _sample = (shift & 1) * envelope_sample();
    }
}
