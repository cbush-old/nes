#include "mappers.h"

SxROM::SxROM(): ROM() {}

SxROM::~SxROM() {}

void SxROM::write_prg(uint8_t value, uint16_t addr) {
  // Only bits 7 and 0 are significant when writing to a register:
  // [r... ...d]
  //    r = reset flag
  //    d = data bit
  bool reset = value & 0x80;
  if (reset) {

    _write = 0;
    uint8_t data = _reg8 | 0xc;
    regw(data, 0x8000);

  } else {

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
  prg_bank.push_back(prg.data());
  prg_bank.push_back(prg.data());

}

void SxROM::set_chr(uint8_t count) {

  chr.resize(count * 0x2000);
  chr_bank.push_back(chr.data());
  chr_bank.push_back(chr.data() + 0x1000);

  regw(0xc, 0x8000);
  regw(0x0, 0xa000);
  regw(0x0, 0xc000);
  regw(0x0, 0xe000);

}

void SxROM::regw(uint8_t value, uint16_t addr) {
  if (addr < 0xa000) {
    _reg8 = value;
    switch (_chr_mode) {
      case 0:
        chr_bank[0] = chr.data() + _regA * 0x1000;
        chr_bank[1] = chr.data() + _regA * 0x1000 + 0x1000;
        break;
      case 1:
        chr_bank[0] = chr.data() + _regA * 0x1000;
        chr_bank[1] = chr.data() + _regC * 0x1000;
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

  } else if (addr < 0xc000) {
    _regA = value;
  } else if (addr < 0xe000) {
    _regC = value;
  } else {
    _regE = value;
  }
}

