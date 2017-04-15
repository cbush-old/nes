#include "dmc_generator.h"

#include "bus.h"

static const uint16_t DMC_RATE[]
{
    //  0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
    428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
};

DMC::DMC(IBus &bus)
    : _bus(bus)
{}

void DMC::update()
{
    if (++_t <= DMC_RATE[_frequency_index])
    {
        return;
    }

    _t = 0;

    if (_interrupt)
    {
        _bus.pull_NMI();
    }

    if (!_silence)
    {
        bool inc = _shift & 1;
        if (inc && _output_level <= 125)
        {
            _output_level += 2;
        }
        else if (!inc && _output_level >= 2)
        {
            _output_level -= 2;
        }
    }

    _shift >>= 1;

    if (!--_bits_remaining)
    {
        _bits_remaining = 8;
        if (!_sample)
        {
            _silence = true;
        }
        else
        {
            _silence = false;
            _shift = _sample;
            _sample = 0;
        }
    }

    if (!_sample)
    {

        if (!_bytes_remaining)
        {
            if (_loop_sample)
            {
                _address = (_sample_address << 13) | 0xC000;
                _bytes_remaining = (sample_length << 11) | 1;
            }
            else if (_enable_IRQ)
            {
                _interrupt = true;
            }
        }

        if (_bytes_remaining)
        {
            // stall CPU for 4 cycles
            _sample = _bus.cpu_read(_address);

            ++_address;
            if (!_address)
            {
                _address |= 0x8000;
            }

            --_bytes_remaining;
        }
    }
    _sample = _output_level;
}

void DMC::disable()
{
    _interrupt = false;
    _bytes_remaining = 0;
}

void DMC::enable()
{
    if (!_bytes_remaining)
    {
        _address = (_sample_address << 13) | 0xC000;
        _bytes_remaining = (sample_length << 11) | 1;
    }
}

void DMC::on_reg3_write(uint8_t value)
{
    reg3 = value;
    _sample_address = value;
}
