#include "apu.h"

#include <cmath>
#include <functional>
#include <set>
#include <thread>

const uint8_t PULSE_WAVES[]
{
    0x40, // 0 1 0 0 0 0 0 0 (12.5%)
    0x60, // 0 1 1 0 0 0 0 0 (25%)
    0x78, // 0 1 1 1 1 0 0 0 (50%)
    0x9f, // 1 0 0 1 1 1 1 1 (25% negated)
};

const uint8_t TRIANGLE_STEPS[]
{
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
};

const uint16_t NOISE_PERIODS[]
{
    4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068,
};

const uint8_t LENGTHS[]
{
    //          0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
    /* 0x0_ */ 10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
    /* 0x1_ */ 12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};

const uint16_t DMC_RATE[]
{
    //  0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xA  0xB  0xC  0xD  0xE  0xF
    428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
};

struct Generator
{
    union {
        // Common
        bit<0, 8> reg0;
        bit<8, 8> reg1;
        bit<16, 8> reg2;
        bit<24, 8> reg3;

        // Pulse and noise
        bit<0, 4> volume;
        bit<0, 4> divider_period;
        bit<4, 1> constant_volume;
        bit<5, 1> length_counter_halt;
        bit<5, 1> envelope_loop;

        // Pulse only
        bit<6, 2> duty;
        bit<8, 3> sweep_shift;
        bit<11, 1> sweep_negative;
        bit<12, 3> sweep_period;
        bit<15, 1> sweep_enabled;

        // Pulse and triangle
        bit<16, 11> timer;
        bit<16, 8> timer_low;
        bit<24, 3> timer_high;

        // Pulse, triangle and noise
        bit<27, 5> length_counter_load;

        // Triangle only
        bit<0, 7> linear_counter_reload_value;
        bit<7, 1> triangle_length_counter_halt;
        bit<7, 1> triangle_control;

        // Noise only
        bit<16, 4> noise_period;
        bit<23, 1> noise_mode;

        // DMC only
        bit<0, 4> frequency_index;
        bit<6, 1> loop_sample;
        bit<7, 1> IRQ_enable;
        bit<8, 7> output_level;
        bit<16, 8> sample_address;
        bit<24, 8> sample_length;
    };

    const std::function<void(uint8_t)> regw[4]
    {
        [this](uint8_t value) { reg0 = value; },
        [this](uint8_t value) { reg1_write(value); },
        [this](uint8_t value) { reg2 = value; },
        [this](uint8_t value) {
            // Writing to reg3 typically has side-effects.
            // Handle it in the subclasses.
            reg3_write(value);
        },
    };

    uint16_t _channel_volume{ 0x2000 };
    uint16_t length_counter{ 0 };

    virtual ~Generator() {}

    virtual void set_channel_volume(uint16_t v)
    {
        _channel_volume = v;
    }

    virtual uint16_t get_channel_volume() const
    {
        return _channel_volume;
    }

    virtual void reg1_write(uint8_t value)
    {
        reg1 = value;
    }
    virtual void reg3_write(uint8_t value) = 0;

    virtual void update() = 0;
    virtual double sample() const = 0;

    // (Pulse and noise) envelope and (triangle) linear counter
    virtual void on_quarter_frame(){};

    // Length counters & sweep units
    virtual void on_half_frame() {}

    virtual void enable()
    {
        _enabled = true;
    }

    virtual void disable()
    {
        _enabled = false;
    }

protected:
    bool _enabled{ true };
};

class Generator_with_length_counter : public Generator
{
protected:
    uint8_t _counter{ 0 };

public:
    ~Generator_with_length_counter() = 0;

public:
    virtual void on_half_frame() override
    {
        clock_length_counter();
    }

    virtual void disable() override
    {
        _enabled = false;
        _counter = 0;
    }

protected:
    virtual bool get_length_counter_halt() const
    {
        return length_counter_halt;
    }

    void reload_length_counter()
    {
        if (_enabled)
        {
            _counter = LENGTHS[length_counter_load];
        }
    }

    bool length_counter_active() const
    {
        return _counter > 0;
    }

    inline void clock_length_counter()
    {
        if (!get_length_counter_halt() && _counter > 0)
        {
            --_counter;
        }
    }
};

Generator_with_length_counter::~Generator_with_length_counter() {}

class Generator_with_envelope : public Generator_with_length_counter
{
private:
    bool _started{ false };
    uint8_t _divider_counter;
    uint8_t _counter{ 15 };

public:
    ~Generator_with_envelope() = 0;

private:
    inline void divider_reload()
    {
        _divider_counter = divider_period;
    }

    inline void divider_clock()
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

    inline void on_divider_output_clock()
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

protected:
    double envelope_sample() const
    {
        return (constant_volume ? volume : _counter) / 15.0;
    }

    virtual void on_quarter_frame()
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

    virtual void start_envelope()
    {
        _started = true;
    }
};

Generator_with_envelope::~Generator_with_envelope() {}

struct Pulse : Generator_with_envelope
{
    ~Pulse() {}

    uint16_t t{ 0 };
    uint8_t shift{ 0 };
    uint16_t _sweep_divider{ 0 };
    uint16_t _target{ 0 };
    bool _silenced{ false };
    bool _sweep_reload{ false };

    void reg3_write(uint8_t value) override
    {
        reg3 = value;
        reload_length_counter();
        shift = 0; // restart sequencer
        _silenced = false;
        start_envelope();
    }

    void reg1_write(uint8_t value) override
    {
        reg1 = value;
        adjust_period();
    }

    void adjust_period()
    {
        _target = timer + (sweep_negative ? -1 : 1) * (timer >> sweep_shift);
        if (timer < 8 || _target > 0x7ff)
        {
            _silenced = true;
        }
    }

    virtual void on_half_frame() override
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

    void update() override
    {
        if (++t > timer)
        {
            t = 0;
            ++shift;
        }
    }

    double sample() const override
    {
        if (_silenced || !length_counter_active())
        {
            return 0.0;
        }

        return bool(PULSE_WAVES[duty] & (1 << (shift % 8))) * envelope_sample();
    }
};

struct Triangle : Generator_with_length_counter
{
    ~Triangle() {}

    uint16_t t{ 0 };
    uint8_t step{ 0 };
    bool _linear_counter_reload{ false };
    uint8_t _linear_counter{ 0 };

    virtual void on_quarter_frame() override
    {
        if (_linear_counter_reload)
        {
            _linear_counter = linear_counter_reload_value;
        }
        else if (_linear_counter > 0)
        {
            --_linear_counter;
        }

        if (triangle_control)
        {
            _linear_counter_reload = false;
        }
    }

    virtual bool get_length_counter_halt() const override
    {
        return triangle_length_counter_halt;
    }

    void reg3_write(uint8_t value) override
    {
        reg3 = value;
        triangle_length_counter_halt = 1;
        reload_length_counter();
        _linear_counter_reload = true;
    }

    void update() override
    {
        if (++t > timer)
        {
            t = 0;
            if (length_counter_active() && _linear_counter > 0)
            {
                step = (step + 1) & 31;
            }
        }
    }

    double sample() const override
    {
        if (!_enabled || !length_counter_active() || timer < 2)
        {
            return 0.0;
        }
        return TRIANGLE_STEPS[step] / 15.0;
    }
};

struct Noise : Generator_with_envelope
{
    ~Noise() {}
    int t{ 0 };
    uint16_t shift{ 1 };

    void reg3_write(uint8_t value)
    {
        reg3 = value;
        reload_length_counter();
        start_envelope();
    }

    // 7 6 5 4 3 2 1 0
    //               ^- bit 14 set to bit 0 XOR...
    //   ^         ^--- - bit 1 if !mode
    //   +------------- - bit 6 if mode
    void update()
    {
        if (++t > NOISE_PERIODS[noise_period])
        {
            t = 0;
            bool feedback = bool(shift & 1) ^ bool(shift & (2 << (noise_mode * 5)));
            shift >>= 1;
            shift &= 0x1fff;
            shift |= feedback << 13;
        }
    }

    double sample() const
    {
        if (!_enabled || !length_counter_active())
        {
            return 0.0;
        }
        return double(shift & 1) * envelope_sample();
    }
};

class DMC : public Generator
{
public:
    DMC(IBus &bus)
        : _bus(bus)
    {
    }
    ~DMC() {}

public:
    virtual void update() override
    {
        if (++_t <= DMC_RATE[frequency_index])
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
            if (inc && output_level <= 125)
            {
                output_level = output_level + 2;
            }
            else if (!inc && output_level >= 2)
            {
                output_level = output_level - 2;
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
                if (loop_sample)
                {
                    _address = (sample_address << 13) | 0xC000;
                    _bytes_remaining = (sample_length << 11) | 1;
                }
                else if (IRQ_enable)
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
    }

    virtual double sample() const override
    {
        return output_level / 127.0;
    }

    virtual void disable() override
    {
        _interrupt = false;
        _bytes_remaining = 0;
    }

    virtual void enable() override
    {
        if (!_bytes_remaining)
        {
            _address = (sample_address << 13) | 0xC000;
            _bytes_remaining = (sample_length << 11) | 1;
        }
    }

protected:
    void reg3_write(uint8_t value) override
    {
        reg3 = value;
        sample_address = value;
    }

private:
    IBus &_bus;
    uint8_t _sample{ 0 };
    uint16_t _address{ 0 };
    uint16_t _bytes_remaining{ 0 };
    uint8_t _bits_remaining{ 1 };
    uint8_t _shift{ 0 };
    uint16_t _t{ 0 };
    bool _interrupt{ false };
    bool _silence{ false };
};

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
    audio->put_sample(sample);
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

