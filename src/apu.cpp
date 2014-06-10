#include "apu.h"
#include <unistd.h>
#include <cmath>

const uint8_t PULSE_WAVES[] {
  0x40, // 0 1 0 0 0 0 0 0 (12.5%)
  0x60, // 0 1 1 0 0 0 0 0 (25%)
  0x78, // 0 1 1 1 1 0 0 0 (50%)
  0x9f, // 1 0 0 1 1 1 1 1 (25% negated)
};

const uint8_t TRIANGLE_STEPS[] {
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
};

struct Envelope {
  bool start_flag { false };
  int divider { 15 };
  mutable int counter { 15 };

  double sample() const {
    if (--counter == -1) {
      counter = 15;
    }
    return counter / (double)divider;
  }
};

struct Pulse {
  uint16_t t { 0 };
  uint8_t shift { 0 };
  Envelope _envelope;

  union {

    bit<0, 8> reg0;
    bit<0, 4> volume;
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

  void update() {
    if (++t > timer) {
      t = 0;
      ++shift;
    }
  }

  double sample() const {
    if (timer < 8) {
      return 0.0;
    }
    double env = constant_volume ? volume / (double)0xf : _envelope.sample();
    return bool(PULSE_WAVES[duty] & (1 << (shift % 8))) * env;
  }
};

struct Triangle {
  uint16_t t { 0 };
  uint8_t step { 0 };

  union {
    bit<0, 8> reg0;
    bit<0, 7> linear_counter_reload;
    bit<7, 1> halt_length_counter;

    bit<8, 8> reg1;
    bit<8, 11> timer;
    bit<8, 8> timer_low;

    bit<16, 8> reg2;
    bit<16, 3> timer_high;
    bit<19, 5> length_counter;
  };

  void write_reg2(uint8_t value) {
    reg2 = value;
    // TODO: sets the linear counter reload flag
  }

  void update() {
    if (++t > timer) {
      t = 0;
      ++step;
    }

    if (!halt_length_counter && length_counter > 0) {
      length_counter = length_counter - 1; // FIXME: this actually needs to stop the note
    }
  }

  double sample() const {
    return TRIANGLE_STEPS[step & 31] / 15.0;
  }

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
    _triangle->update();
  }

  int16_t sample = 
      (_pulse1->sample() * 0x2000 - 0x1000)
    + (_pulse2->sample() * 0x2000 - 0x1000)
    + (_triangle->sample() * 0x2000 - 0x1000);

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
    case 0xb: _triangle->write_reg2(value); break;

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

