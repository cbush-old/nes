#ifndef APU_H
#define APU_H

#include <iostream>
#include <vector>
#include <cstdint>
#include <array>

#include "bus.h"
#include "bit.h"
#include "async_component.h"

class APU : public IAPU {
  private:
    IBus *bus;
    IAudioDevice *audio { nullptr };

  public:
    APU(IBus *bus, IAudioDevice *audio);
    ~APU();

  public:
    uint8_t read() const;
    void write(uint8_t, uint8_t);

  protected:
    virtual uint32_t CLOCK_FREQUENCY_HZ() const override { return 1789773; };
    void on_event(IEvent const& e) override;

  protected:
    void tick() override;

  public:
    State dummy; // temp
    State const& get_state() const { return dummy; }
    void set_state(State const&) {}

  private:
    std::array<struct Generator *, 5> _generator;

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

    uint16_t _cycle { 0 };

};

#endif
