#ifndef BUS_H
#define BUS_H

#include <string>
#include <vector>

namespace bus {

  // Memory read
  uint8_t ppu_reg_read(uint8_t);
  uint8_t apu_read();
  uint8_t io_input_state(uint8_t);
  uint8_t& rom_memref(uint16_t);
  uint8_t& rom_nt(uint8_t, uint16_t);
  uint8_t& rom_vbank(uint16_t);

  // Memory write
  void ppu_reg_write(uint8_t, uint8_t);
  void apu_write(uint8_t, uint16_t);
  void rom_write(uint8_t, uint16_t);

  // Debug
  uint8_t debug_ppu_get_scanline();

  // Tick
  void ppu_tick3();
  void apu_tick();
  void io_swap_with(std::vector<uint32_t> const&);
  void io_strobe();
  uint8_t io_handle_input();

  void play(std::string const&);
  void start();

  uint8_t cpu_read(uint16_t);
  void pull_NMI();
  void pull_IRQ();
  void reset_IRQ();
  
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
