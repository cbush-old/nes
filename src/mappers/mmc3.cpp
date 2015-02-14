#include "mmc3.h"
#include <iostream>

const int PRG_PAGE_SIZE = 0x2000;
const int CHR_PAGE_SIZE = 0x400;

MMC3::MMC3(IBus *bus)
  : _bus(bus)
{
  for (int i = 0; i < 8; ++i) {
    std::cout << "MMC3 R" << i << ": " << (int)_reg8001[i] << "\n";
  }
}

MMC3::~MMC3(){}

void MMC3::write_chr(uint8_t value, uint16_t addr) {
  // TODO (?)
}

uint8_t MMC3::read_prg(uint16_t addr) const {
  if (addr < 0x6000) {
    // ??
    std::cerr << "MMC3 read from " << std::hex << addr << "\n";
    return 0;

  } else if (addr < 0x8000) {

    return _ram[addr % 0x2000];

  } else {
    addr -= 0x8000;
    return prg_bank[addr / 0x2000][addr % 0x2000];

  }
}


uint8_t MMC3::read_chr(uint16_t addr) const {
  if ((addr & 0x1000) && !_edge) {
    _edge = true;

    if (_IRQ_counter == 0) {

      _IRQ_counter = _IRQ_reload;

    } else if (--_IRQ_counter == 0 && _IRQ_enabled) {
      _IRQ_pending = true;
      _bus->pull_IRQ();
      std::cout << "mmc3 pull irq\n";
    }

    _edge_counter = 0;

  } else if (!(addr & 0x1000)) {
    _edge = false;
  }

  if (addr > 0x1fff) {
    return 0;
  }

  return chr_bank.at(addr / 0x400)[addr % 0x400];

}

void MMC3::set_prg(uint8_t count) {
  std::cout << "MMC set prg\n";
  prg.resize(count * 0x4000);
  std::cout << std::hex << prg.size() << "<\n";
  prg_bank.push_back(prg.data());
  prg_bank.push_back(prg.data());
  prg_bank.push_back(prg.data());
  prg_bank.push_back(prg.data() + prg.size() - PRG_PAGE_SIZE);
}

void MMC3::set_chr(uint8_t count) {
  if (!count) {
    count = 0x2000 / CHR_PAGE_SIZE;
  }
  chr.resize(count * 0x2000);
  chr_bank.push_back(chr.data());
  chr_bank.push_back(chr.data());
  chr_bank.push_back(chr.data());
  chr_bank.push_back(chr.data());
  chr_bank.push_back(chr.data());
  chr_bank.push_back(chr.data());
  chr_bank.push_back(chr.data());
  chr_bank.push_back(chr.data());

  write_prg(0, 0x8000);
  write_prg(0, 0x8001);
  write_prg(0, 0xa000);
  write_prg(0, 0xa001);
  write_prg(0, 0xc000);
  write_prg(0, 0xc001);
  write_prg(0, 0xe000);
  write_prg(0, 0xe001);

  setup();

}

void MMC3::write_prg(uint8_t value, uint16_t addr) {
  if (addr < 0x6000) {
    // ??
    std::cerr << "MMC3 write to " << std::hex << addr << " (" << (int)value << ")\n";
    return;

  } else if (addr < 0x8000) {

    _ram[addr % 0x2000] = value;

  } else {
    switch (addr & 0xe001) {
      case 0x8000:
        _reg8000 = value;
        break;

      case 0x8001:
        _reg8001.at(_address) = value;
        if (_address <= 1) {
          _reg8001.at(_address) &= 0xfe;
        } else if (_address >= 6) {
          _reg8001.at(_address) &= ~0xc0;
        }
        break;

      case 0xa000:
        _mirroring = value & 1;
        set_mirroring(
          _mirroring ? HORIZONTAL : VERTICAL
        );
        break;

      case 0xa001:
        _regA001 = value;
        break;

      case 0xc000:
        _IRQ_reload = value;
        break;

      case 0xc001:
        _IRQ_counter = 0;
        break;

      case 0xe000:
        _IRQ_enabled = false;
        _IRQ_pending = false;
        break;

      case 0xe001:
        _IRQ_enabled = true;
        break;
    }
  }
  setup();
}

void MMC3::setup() {
  std::array<uint8_t, 8>& R = _reg8001;
  if (!_prg_mode_1) {
    prg_bank[0] = prg.data() + R[6] * PRG_PAGE_SIZE;
    prg_bank[1] = prg.data() + R[7] * PRG_PAGE_SIZE;
    prg_bank[2] = prg.data() + prg.size() - PRG_PAGE_SIZE * 2;
    prg_bank[3] = prg.data() + prg.size() - PRG_PAGE_SIZE;
  } else {
    prg_bank[0] = prg.data() + prg.size() - PRG_PAGE_SIZE * 2;
    prg_bank[1] = prg.data() + R[7] * PRG_PAGE_SIZE;
    prg_bank[2] = prg.data() + R[6] * PRG_PAGE_SIZE;
    prg_bank[3] = prg.data() + prg.size() - PRG_PAGE_SIZE;
  }

  auto R0_a = R[0] & 0xfe;
  auto R0_b = R[0] | 1;
  auto R1_a = R[1] & 0xfe;
  auto R1_b = R[1] | 1;
  if (!_chr_mode_1) {
    chr_bank[0] = chr.data() + R0_a * CHR_PAGE_SIZE;
    chr_bank[1] = chr.data() + R0_b * CHR_PAGE_SIZE;
    chr_bank[2] = chr.data() + R1_a * CHR_PAGE_SIZE;
    chr_bank[3] = chr.data() + R1_b * CHR_PAGE_SIZE;
    chr_bank[4] = chr.data() + R[2] * CHR_PAGE_SIZE;
    chr_bank[5] = chr.data() + R[3] * CHR_PAGE_SIZE;
    chr_bank[6] = chr.data() + R[4] * CHR_PAGE_SIZE;
    chr_bank[7] = chr.data() + R[5] * CHR_PAGE_SIZE;
  } else {
    chr_bank[0] = chr.data() + R[2] * CHR_PAGE_SIZE;
    chr_bank[1] = chr.data() + R[3] * CHR_PAGE_SIZE;
    chr_bank[2] = chr.data() + R[4] * CHR_PAGE_SIZE;
    chr_bank[3] = chr.data() + R[5] * CHR_PAGE_SIZE;
    chr_bank[4] = chr.data() + R0_a * CHR_PAGE_SIZE;
    chr_bank[5] = chr.data() + R0_b * CHR_PAGE_SIZE;
    chr_bank[6] = chr.data() + R1_a * CHR_PAGE_SIZE;
    chr_bank[7] = chr.data() + R1_b * CHR_PAGE_SIZE;
  }
}
