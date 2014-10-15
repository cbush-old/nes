#include "mappers.h"

#include <iostream>

SxROM::SxROM(): ROM() {}

SxROM::~SxROM() {}

void SxROM::write_prg(uint8_t value, uint16_t addr) {
  if (addr < 0x8000) {
    ram[addr % 0x4000] = value;
    return;
  }

  // Only bits 7 and 0 are significant when writing to a register:
  // [r... ...d]
  //    r = reset flag
  //    d = data bit
  if (value & 0x80) { // reset

    _register = 0;
    _write = 0;
    regw(_reg8 | 0xc, 0x8000);

  } else { // data

    _register |= (value & 1) << (_write++);
    if (_write == 5) {
      regw(_register, addr);
      _register = 0;
      _write = 0;
    }

  }
}

void SxROM::set_prg(uint8_t count) {
  prg.resize(count * 0x4000);
  std::cout << std::hex << prg.size() << "<\n";
  prg_bank.push_back(prg.data());
  prg_bank.push_back(prg.data());
}

void SxROM::set_chr(uint8_t count) {

  chr.resize(count ? count * 0x2000 : 0x4000);
  std::cout << std::hex << "prg: " << prg.size() << "<\n";
  chr_bank.push_back(chr.data());
  chr_bank.push_back(chr.data() + 0x1000);

  regw(0xff, 0xe000);
  regw(0xff, 0xc000);
  regw(0xff, 0xa000);
  regw(0x0c, 0x8000);

}

void SxROM::regw(uint8_t value, uint16_t addr) {

  if (addr < 0xa000) {

    _reg8 = value;

    ROM::MirrorMode mirror;
    switch (_mirror_mode) {
      case 0: mirror = ROM::MirrorMode::SINGLE_SCREEN_A; break;
      case 1: mirror = ROM::MirrorMode::SINGLE_SCREEN_B; break;
      case 2: mirror = ROM::MirrorMode::VERTICAL; break;
      case 3: mirror = ROM::MirrorMode::HORIZONTAL; break;
    }
    set_mirroring(mirror);

  } else if (addr < 0xc000) {
    _regA = value;
  } else if (addr < 0xe000) {
    _regC = value;
  } else {
    _regE = value;
  }

  switch (_chr_mode) {
    case 0:
      chr_bank[0] = chr.data() + _regA * 0x1000;
      chr_bank[1] = chr.data() + _regA * 0x1000 + 0x800;
      break;
    case 1:
      chr_bank[0] = chr.data() + _regA * 0x1000;
      chr_bank[1] = chr.data() + _regC * 0x1000;
      break;
  }

  const uint16_t size = 0x4000;
  switch (_prg_size) {
    case 0:
      prg_bank[0] = prg.data() + (_prg_reg & ~1) * size;
      prg_bank[1] = prg.data() + (_prg_reg & ~1) * size + size;
      break;

    case 1:
      if (!_slot_select) {
        prg_bank[0] = prg.data();
        prg_bank[1] = prg.data() + _prg_reg * size;
      } else {
        prg_bank[0] = prg.data() + _prg_reg * size;
        prg_bank[1] = prg.data() + prg.size() - size;
      }
      break;
  }

}

