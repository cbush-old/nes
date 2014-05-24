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

};

#endif
