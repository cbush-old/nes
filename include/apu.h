#ifndef APU_H
#define APU_H

#include <iostream>
#include <vector>
#include <cstdint>
#include "bus.h"
#include "bit.h"

class APU : public IAPU {
  private:
    IBus *bus;

  public:
    APU(IBus *bus);
    ~APU();

  public:
    void tick();

  public:
    uint8_t read() const;
    void write(uint8_t, uint8_t);

  public:
    State dummy; // temp
    State const& get_state() const { return dummy; }
    void set_state(State const&) {}

  private:
    struct Pulse *_pulse1, *_pulse2;
    struct Triangle *_triangle;
    struct Noise *_noise;
    struct DMC *_dmc;

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
      bit<6, 1, uint8_t> disable_frame_interrupt;
      bit<7, 1, uint8_t> five_frame_sequence;
    } _frame_counter;
};

#endif
