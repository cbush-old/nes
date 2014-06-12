#include "mappers.h"

#include <iostream>

SxROM::SxROM(): ROM() {}

SxROM::~SxROM() {}

void SxROM::write_prg(uint8_t value, uint16_t addr) {
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

  chr.resize(0x4000 + count * 0x2000);
  chr_bank.push_back(chr.data());
  chr_bank.push_back(chr.data() + 0x1000);

  regw(0x00, 0xe000);
  regw(0x00, 0xc000);
  regw(0x00, 0xa000);
  regw(0x0c, 0x8000);

}

void SxROM::regw(uint8_t value, uint16_t addr) {
  if (addr < 0x8000) {
    // ??
    std::cerr << "Writing to unexpected place on SxROM: " << addr << "\n";
  } else if (addr < 0xa000) {
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
      chr_bank[0] = chr.data() + _regA * 0x2000;
      chr_bank[1] = chr.data() + _regA * 0x2000 + 0x1000;
      break;
    case 1:
      chr_bank[0] = chr.data() + _regA * 0x2000;
      chr_bank[1] = chr.data() + _regC * 0x2000;
      break;
  }

  switch (_prg_size) {
    case 0:
      prg_bank[0] = prg.data() + _prg_reg * 0x4000;
      prg_bank[1] = prg.data() + _prg_reg * 0x4000 + 0x4000;
      break;

    case 1:
      if (!_slot_select) {
        prg_bank[0] = prg.data();
        prg_bank[1] = prg.data() + _prg_reg * 0x4000;
      } else {
        prg_bank[0] = prg.data() + _prg_reg * 0x4000;
        prg_bank[1] = prg.data() + 0xf * 0x4000;
      }
      break;
  }

}

