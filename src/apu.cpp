#include "apu.h"

#include <cmath>
#include <functional>
#include <set>
#include <thread>

APU::APU(IBus *bus, IAudioDevice *audio)
    : bus(bus)
    , audio(audio)
    , _dmc(*bus)
    , _generators
    {
        &_pulse[0],
        &_pulse[1],
        &_triangle,
        &_noise,
        &_dmc,
    }
{
    write(0, 0x17);
}

APU::~APU() = default;

void APU::on_quarter_frame()
{
    _pulse[0].on_quarter_frame();
    _pulse[1].on_quarter_frame();
    _triangle.on_quarter_frame();
    _noise.on_quarter_frame();
}

void APU::on_half_frame()
{
    _pulse[0].on_half_frame();
    _pulse[1].on_half_frame();
    _triangle.on_half_frame();
    _noise.on_half_frame();
}

void APU::tick()
{
    // Update generators on every other CPU cycle
    if (_cycle & 1)
    {
        _pulse[0].update();
        _pulse[1].update();
        _noise.update();
        _dmc.update();
    }

    // triangle is updated at CPU rate
    _triangle.update();

    ++_cycle;

    if (!_frame_counter.five_frame_sequence)
    {
        tick_four_frame();
    }
    else
    {
        tick_five_frame();
    }

    int16_t sample = mix();

    audio->put_sample(sample, 1'789'773);
}

void APU::tick_four_frame()
{
    if ((_cycle == 3728.5 * 2) || (_cycle == 11185.5 * 2))
    {
        on_quarter_frame();
    }
    else if (_cycle == 7456.5 * 2)
    {
        on_quarter_frame();
        on_half_frame();
    }
    else if (_cycle == 14914 * 2)
    {
        if (!_frame_counter.interrupt_inhibit)
        {
            bus->pull_IRQ();
        }
    }
    else if (_cycle == 14914.5 * 2)
    {
        on_quarter_frame();
        on_half_frame();

        if (!_frame_counter.interrupt_inhibit)
        {
            bus->pull_IRQ();
        }
    }
    else if (_cycle >= 14915 * 2)
    {
        if (!_frame_counter.interrupt_inhibit)
        {
            bus->pull_IRQ();
        }
        _cycle = 0;
    }
}

void APU::tick_five_frame()
{
    // five-step sequence
    const bool quarter_frame = (_cycle == 3728.5 * 2) || (_cycle == 7456.5 * 2) || (_cycle == 11185.5 * 2) || (_cycle == 18640.5 * 2);

    const bool half_frame = (_cycle == 7456.5 * 2) || (_cycle == 18640.5 * 2);

    if (quarter_frame)
    {
        on_quarter_frame();
    }
    
    if (half_frame)
    {
        on_half_frame();
    }

    if (_cycle >= 18641)
    {
        _cycle = 0;
    }
}

int16_t APU::mix() const
{
    int16_t sample = 0;
    sample += _pulse[0].mixed_sample();
    sample += _pulse[1].mixed_sample();
    sample += _triangle.mixed_sample();
    sample += _noise.mixed_sample();
    sample += _dmc.mixed_sample();
    return sample;
}

uint8_t APU::read() const
{
    return _status.data & ~0x20;
}

void APU::write(uint8_t value, uint8_t index)
{
    if (index < 0x14)
    {
        _generators[index / 4]->regw(index, value);
    }
    else if (index == 0x15)
    {
        _status.control = value;

        value &= 0x1f;
        for (int i = 0; i < 5; ++i)
        {
            if (value & (1 << i))
            {
                _generators[i]->enable();
            }
            else
            {
                _generators[i]->disable();
            }
        }
    }
    else if (index == 0x17)
    {
        _frame_counter.data = value & 0xc0;

        if (_frame_counter.interrupt_inhibit)
        {
            bus->release_IRQ();
        }

        // FIXME: this actually happens 3-4 cycles after write...
        _cycle = 0;

        if (_frame_counter.five_frame_sequence)
        {
            on_quarter_frame();
            on_half_frame();
        }
    }
}

