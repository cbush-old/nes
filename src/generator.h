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

    // reg0 // bit_index, bit_count
    union
    {
        uint8_t _volume; // 0, 4
        uint8_t _divider_period;
        uint8_t _frequency_index;
    };
    bool _constant_volume; // 4, 1
    union
    {
        bool _length_counter_halt; // 5, 1
        bool _envelope_loop;
    };
    bool _loop_sample; // 6, 1
    uint8_t _duty; // 6, 2
    uint8_t _linear_counter_reload_value; // 0, 7
    union
    {
        bool _triangle_length_counter_halt; // 7, 1
        bool _triangle_control;
        bool _enable_IRQ;
    };
    
    // reg2
    union
    {
        uint8_t _timer_low; // 16, 8
        uint8_t _sample_address;
    };
    uint16_t _timer; // 16, 11
    uint8_t _noise_period; // 16, 4
    bool _noise_mode; // 23, 1

    union
    {
        // Common
        bit<8, 8> reg1;
        bit<24, 8> reg3;

        // Pulse only
        bit<8, 3> sweep_shift;
        bit<11, 1> sweep_negative;
        bit<12, 3> sweep_period;
        bit<15, 1> sweep_enabled;

        // Pulse and triangle
        bit<24, 3> timer_high;

        // Pulse, triangle and noise
        bit<27, 5> length_counter_load;

        // DMC only
        bit<8, 7> output_level;
        bit<24, 8> sample_length;
    };

    bool _enabled{ true };
    uint8_t _sample{0};

protected:
    void set_timer(uint16_t value);

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
