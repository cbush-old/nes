#pragma once

#include "bit.h"

#include <functional>

struct Generator
{
public:
    void regw(size_t r, uint8_t value);

    void set_channel_volume(int16_t v);

    int16_t get_channel_volume() const;

    int16_t mixed_sample() const;

    virtual void enable();

    virtual void disable();

protected:
    ~Generator();

    union
    {
        // Common
        bit<0, 8> reg0{0};
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

    bool _enabled{ true };
    uint8_t _sample{0};
    
private:
    virtual void reg1_write(uint8_t value);

    virtual void reg3_write(uint8_t value) = 0;

    int16_t _channel_volume{0x100};
};

class Generator_with_length_counter : public Generator
{
public:
    void on_half_frame();

    virtual void disable() override;

protected:
    ~Generator_with_length_counter();

    virtual bool get_length_counter_halt() const;

    void reload_length_counter();

    bool length_counter_active() const;

    void clock_length_counter();

private:
    uint8_t _counter{0};
};

class Generator_with_envelope : public Generator_with_length_counter
{
public:
    void divider_reload();
    void divider_clock();
    void on_divider_output_clock();
    void on_quarter_frame();

protected:
    ~Generator_with_envelope();
    
    uint8_t envelope_sample() const;
    void start_envelope();

private:
    bool _started{ false };
    uint8_t _divider_counter;
    uint8_t _counter{ 15 };
};
