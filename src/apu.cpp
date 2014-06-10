#include "apu.h"
#include <unistd.h>
#include <cmath>

const uint8_t pulse_waves[] {
  0x40, // 0 1 0 0 0 0 0 0 (12.5%)
  0x60, // 0 1 1 0 0 0 0 0 (25%)
  0x78, // 0 1 1 1 1 0 0 0 (50%)
  0x9f, // 1 0 0 1 1 1 1 1 (25% negated)
};

struct Envelope {
  bool start_flag { false };
  int divider { 1 };
  int t { 0 }; 
};

struct Pulse {
  uint16_t t { 0 };
  uint8_t shift { 0 };
  union {

    bit<0, 8> reg0;
    bit<0, 4> envelope;
    bit<4, 1> constant_volume;
    bit<5, 1> length_counter_disabled;
    bit<6, 2> duty;

    bit<8, 8> reg1;
    bit<8, 3> sweep_shift_count;
    bit<11, 1> sweep_negative;
    bit<12, 3> sweep_period;
    bit<15, 1> sweep_enabled;

    bit<16, 8> reg2;
    bit<16, 11> timer;
    bit<16, 8> timer_low;

    bit<24, 8> reg3;
    bit<24, 3> timer_high;
    bit<27, 5> length_counter_load;

  };

  void reg3_write(uint8_t value) {
    reg3 = value;
    // Side effect: the sequencer and envelope are restarted
    shift = 0;
  }

  int counter { 0 };

  void update() {
    if (++t > timer) {
      t = 0;
      ++shift;
    }
  }

  bool sample() const {
    return timer > 8 && (pulse_waves[duty] & (1 << (shift % 8)));
  }
};

struct Triangle {

  union {
    bit<0, 8> reg0;
    bit<0, 7> linear_counter_reload;
    bit<7, 1> length_counter_disabled;

    bit<8, 8> reg1;
    bit<8, 11> timer;
    bit<8, 8> timer_low;

    bit<16, 8> reg2;
    bit<16, 3> timer_high;
    bit<19, 5> length_counter_load;
  };

};

struct Noise {
  // TODO
};

struct DMC {
  // TODO
};

FILE *fp;

APU::APU(IBus *bus)
  : bus(bus)
  , _pulse1(new Pulse())
  , _pulse2(new Pulse())
  , _triangle(new Triangle())
  , _noise(new Noise())
  , _dmc(new DMC())
{
  fp = popen(
    "sox -t raw -c1 -b 16 -e signed-integer -r1789773 - -t raw -c2 - rate 48000 "
    " | aplay -fdat",
    "w"
  );
}

APU::~APU(){
  pclose(fp);
  delete _pulse1;
  delete _pulse2;
  delete _triangle;
  delete _noise;
  delete _dmc;
}

bool other = false;

void APU::tick() {
  if (other ^= 1) {
    _pulse1->update();
    _pulse2->update();
  }

  int16_t sample = 
      (_pulse1->sample() * 0x1fff - 0x1000)
    + (_pulse2->sample() * 0x1fff - 0x1000);

  // test
  // static double i = 0;
  // sample = 0x0 + 0x7fff * sin(i += 0.001);

  fputc(sample, fp);
  fputc(sample >> 8, fp);



}

uint8_t APU::read() const {
  return _status.data;
}

void APU::write(uint8_t value, uint8_t index) {
  switch (index % 0x18) {
    case 0x0: _pulse1->reg0 = value; break;
    case 0x1: _pulse1->reg1 = value; break;
    case 0x2: _pulse1->reg2 = value; break;
    case 0x3:
      _pulse1->reg3_write(value);
      break;

    case 0x4: _pulse2->reg0 = value; break;
    case 0x5: _pulse2->reg1 = value; break;
    case 0x6: _pulse2->reg2 = value; break;
    case 0x7:
      _pulse2->reg3_write(value);
      // reset duty and start envelope
      // TODO
      break;

    case 0x8: _triangle->reg0 = value; break;
    case 0xa: _triangle->reg1 = value; break;
    case 0xb:
      _triangle->reg2 = value;
      // also reloads linear counter
      // TODO
      break;

    case 0xc:
    case 0xe:
    case 0xf:
      // noise
      // TODO
      break;

    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
      // dmc
      // TODO
      break;

    case 0x15:
      _status.control = value;
      break;

    case 0x17:
      _frame_counter.data = value & 0xc0;
  }
}

