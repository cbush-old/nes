#ifndef APU_H
#define APU_H

#include <iostream>
#include <vector>
#include <cstdint>
#include <array>

#include "bus.h"
#include "bit.h"
#include "dmc_generator.h"
#include "noise_generator.h"
#include "pulse_generator.h"
#include "triangle_generator.h"

class APU : public IAPU
{
public:
    APU(IBus *bus, IAudioDevice *audio);
    ~APU();

    virtual void tick(double rate) override;

    virtual uint8_t read() const override;

    virtual void write(uint8_t, uint8_t) override;

private:
    void tick_four_frame();
    void tick_five_frame();
    int16_t mix() const;
    
    void on_quarter_frame();
    void on_half_frame();

    IBus *bus;
    IAudioDevice *audio{ nullptr };

    Pulse _pulse[2];
    Triangle _triangle;
    Noise _noise;
    DMC _dmc;
    std::array<Generator *, 5> _generators;

    union {
        uint8_t data;
        bit<0, 5, uint8_t> control;
        bit<0, 1, uint8_t> pulse1;
        bit<1, 1, uint8_t> pulse2;
        bit<2, 1, uint8_t> triangle;
        bit<3, 1, uint8_t> noise;
        bit<4, 1, uint8_t> dmc;
        bit<6, 1, uint8_t> frame_interrupt;
        bit<7, 1, uint8_t> dmc_interrupt;
    } _status;

    union {
        uint8_t data;
        bit<6, 1, uint8_t> interrupt_inhibit;
        bit<7, 1, uint8_t> five_frame_sequence;
    } _frame_counter;

    uint16_t _cycle{ 0 };
};

#endif
