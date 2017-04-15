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

    // reg1
    uint8_t _sweep_shift; // 0, 3
    bool _sweep_negative; // 4, 1
    uint8_t _sweep_period; // 5, 3
    bool _sweep_enabled; // 7, 1
    uint8_t _output_level; // 0, 7

    // reg2
    union
    {
        uint8_t _timer_low; // 0, 8
        uint8_t _sample_address;
    };
    uint16_t _timer; // 0, 11
    uint8_t _noise_period; // 0, 4
    bool _noise_mode; // 7, 1

    // reg3
    // Pulse and triangle
    uint8_t _timer_high; // 0, 3

    // Pulse, triangle and noise
    uint8_t _length_counter_load; // 3, 5

    // DMC only
    uint32_t _sample_length; // 0, 8

    bool _enabled{ true };
    uint8_t _sample{0};

protected:
    void set_timer(uint16_t value);

private:
    virtual void on_reg1_write(uint8_t value);

    virtual void on_reg3_write(uint8_t value);

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
