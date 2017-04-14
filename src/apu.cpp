#include "apu.h"

#include "dmc_generator.h"
#include "noise_generator.h"
#include "pulse_generator.h"
#include "triangle_generator.h"

#include <cmath>
#include <functional>
#include <set>
#include <thread>

APU::APU(IBus *bus, IAudioDevice *audio)
    : bus(bus)
    , audio(audio)
    , _generator{
        { new Pulse(),
          new Pulse(),
          new Triangle(),
          new Noise(),
          new DMC(*bus) }
    }
{
    write(0, 0x17);
}

APU::~APU()
{
    for (int i = 0; i < 5; ++i)
    {
        delete _generator[i];
    }
}

bool other = false;

void APU::tick()
{

    // Update generators on every other CPU cycle
    if (_cycle & 1)
    {
        for (auto &g : _generator)
        {
            g->update();
        }
    }
    else
    {

        // triangle is updated at CPU rate
        _generator[2]->update();
    }

    ++_cycle;

    if (!_frame_counter.five_frame_sequence)
    {

        if ((_cycle == 3728.5 * 2) || (_cycle == 11185.5 * 2))
        {

            for (auto &g : _generator)
            {
                g->on_quarter_frame();
            }
        }
        else if (_cycle == 7456.5 * 2)
        {

            for (auto &g : _generator)
            {
                g->on_quarter_frame();
                g->on_half_frame();
            }
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

            for (auto &g : _generator)
            {
                g->on_quarter_frame();
                g->on_half_frame();
            }

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
    else
    { // five-step sequence
        bool quarter_frame = (_cycle == 3728.5 * 2) || (_cycle == 7456.5 * 2) || (_cycle == 11185.5 * 2) || (_cycle == 18640.5 * 2);

        bool half_frame = (_cycle == 7456.5 * 2) || (_cycle == 18640.5 * 2);

        if (quarter_frame || half_frame)
        {
            for (auto &g : _generator)
            {
                if (quarter_frame)
                    g->on_quarter_frame();
                if (half_frame)
                    g->on_half_frame();
            }
        }

        if (_cycle >= 18641)
        {
            _cycle = 0;
        }
    }

    // Mix sample

    int16_t sample = 0;

    for (auto &g : _generator)
    {
        sample += (g->sample() - 0.5) * g->get_channel_volume();
    }

    //sample += _generator[4]->sample() - 0.5;

    /*
  int samples = 1;
  double rate = bus->get_rate();

  static int s = 0, t = 0; // FIXME: make member

  if (rate < 1.0) {
    samples = 1 / rate;
    double remainder = fmod(1, rate);
    if (remainder > 0.0001) {
      if (++s > (int)(1.0 / remainder)) {
        s = 0;
        ++samples;
      }
    }
  } else if (rate > 1.0) {
    // output a sample every 1/1/rate ticks
    samples = 0;
    double remainder = fmod(1.0, 1.0 / rate);
    if (s++ > 1.0 / (1.0 / rate)) {
      s = 0;
      samples = 1;
      if (remainder > 0.0001 && (t++ > (1.0 / remainder))) {
        t = 0;
        samples = 0;
      }
    }
  }
  for (int i = 0; i < samples; ++i)
  */
    (void)audio;
    //audio->put_sample(sample);
}

uint8_t APU::read() const
{
    return _status.data & ~0x20;
}

void APU::write(uint8_t value, uint8_t index)
{
    if (index < 0x14)
    {

        _generator[index / 4]->regw[index & 3](value);
    }
    else if (index == 0x15)
    {

        _status.control = value;

        value &= 0x1f;
        for (int i = 0; i < 5; ++i)
        {
            if (value & (1 << i))
            {
                _generator[i]->enable();
            }
            else
            {
                _generator[i]->disable();
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
            for (auto &g : _generator)
            {
                g->on_quarter_frame();
                g->on_half_frame();
            }
        }
    }
}

