#ifndef BUS_H
#define BUS_H

#include <string>
#include <vector>

class ROM;
class CPU;
class PPU;
class APU;
class IO;

namespace bus {
  
  CPU& cpu();
  ROM& rom();
  PPU& ppu();
  APU& apu();
  IO& io();
  
  uint8_t cpu_read(uint16_t);
  void pull_NMI();
  void pull_IRQ();
  void reset_IRQ();
  void play(std::string const&);
  
  void save_state();
  void restore_state();
  
}

struct State {

  uint8_t P, A, X, Y, SP;
  uint16_t PC;
  int result_cycle;

  std::vector<uint8_t> cpu_memory, ppu_memory, palette;

  uint32_t ppu_reg, scroll, vram;
  
  int read_buffer, vblank_state;
  bool loopy_w, NMI_pulled;
  
  State():
    cpu_memory(0x800, 0xff),
    ppu_memory(0x800),
    palette(0x20)
    {}

};

namespace bus {
  void get_state(State&);
  void restore_state(State const&);
}

#endif
