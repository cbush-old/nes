#pragma once

#include "bit.h"

#include <functional>

struct Generator
{
public:
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

    virtual ~Generator();

    virtual void set_channel_volume(uint16_t v);

    virtual uint16_t get_channel_volume() const;

    virtual void reg1_write(uint8_t value);

    virtual void reg3_write(uint8_t value) = 0;

    virtual void update() = 0;
    virtual double sample() const = 0;

    // (Pulse and noise) envelope and (triangle) linear counter
    virtual void on_quarter_frame();

    // Length counters & sweep units
    virtual void on_half_frame();

    virtual void enable();

    virtual void disable();

protected:
    bool _enabled{ true };
};

class Generator_with_length_counter : public Generator
{
public:
    ~Generator_with_length_counter() = 0;

    virtual void on_half_frame() override;
    virtual void disable() override;

protected:
    virtual bool get_length_counter_halt() const;

    void reload_length_counter();

    bool length_counter_active() const;

    void clock_length_counter();

    uint8_t _counter{ 0 };
};

class Generator_with_envelope : public Generator_with_length_counter
{
private:
    bool _started{ false };
    uint8_t _divider_counter;
    uint8_t _counter{ 15 };

public:
    ~Generator_with_envelope() = 0;

    void divider_reload();
    void divider_clock();
    void on_divider_output_clock();

protected:
    double envelope_sample() const;
    virtual void on_quarter_frame();
    virtual void start_envelope();
};

