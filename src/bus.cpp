#include "bus.h"
#include "rom.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"

namespace bus {

IO _io;
PPU _ppu;
CPU _cpu;
APU _apu;
ROM *_rom;

// Memory read
uint8_t ppu_reg_read(uint8_t reg) {
  return _ppu.reg_read(reg);
}

uint8_t apu_read() {
  return _apu.read();
}

uint8_t io_input_state(uint8_t i) {
  return _io.input_state(i);
}

uint8_t& rom_memref(uint16_t addr) {
  return _rom->memref(addr);
}

uint8_t& rom_nt(uint8_t table, uint16_t addr) {
  return _rom->nt(table, addr);
}

uint8_t& rom_vbank(uint16_t addr) {
  return _rom->vbank(addr);
}

void ppu_reg_write(uint8_t value, uint8_t reg) {
  _ppu.reg_write(reg, value);
}

void apu_write(uint8_t value, uint16_t addr) {
  _apu.write(value, addr);
}

void rom_write(uint8_t value, uint16_t addr) {
  _rom->write(value, addr);
}

void io_strobe() {
  _io.strobe();
}

uint8_t debug_ppu_get_scanline() {
  return _ppu.debug_get_scanline();
}

void io_swap_with(std::vector<uint32_t> const& raster) {
  _io.swap_with(raster);
}

uint8_t io_handle_input() {
  return _io.handle_input();
}

void ppu_tick3() {
  for (int i = 0; i < 3; ++i) {
    _ppu.tick();
  }
}
void apu_tick() {
  _apu.tick();
}

void pull_NMI() {
  _cpu.pull_NMI();
}

void play(std::string const& path) {
  _rom = new ROM(path);
}

State state1;

void save_state(){
  _ppu.save_state(state1);
  _cpu.save_state(state1);
}

void restore_state(){
  _ppu.load_state(state1);
  _cpu.load_state(state1);
}

void restore_state(State const& s){
  _ppu.load_state(s);
  _cpu.load_state(s);
}

void get_state(State& s){
  _ppu.save_state(s);
  _cpu.save_state(s);
}

void pull_IRQ(){
  _cpu.IRQ = 0;
}

void reset_IRQ(){
  _cpu.IRQ = 1;
}

uint8_t cpu_read(uint16_t addr){
  return _cpu.read(addr);
}

void start() {
  _cpu.run();
}

}

