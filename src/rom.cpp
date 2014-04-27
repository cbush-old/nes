#include "rom.h"
#include "bit.h"

#include <iostream>
#include <fstream>


// CPU-space access
//
//
uint8_t dummy;
uint8_t& ROM::getmemref(uint16_t addr) {
  if (addr < 0x8000) {
    return dummy;
  } else {
    addr -= 0x8000;
    return prg_bank[addr / 0x4000][addr & 0x3fff];
  }
}

uint8_t ROM::read_prg(uint16_t addr) const {
  if (addr < 0x8000) {
    // TODO
    return 0;
  } else {
    addr -= 0x8000;
    return prg_bank[addr / 0x4000][addr & 0x3fff];
  }
}

void ROM::write_prg(uint8_t value, uint16_t addr) {
  if (addr < 0x8000) {
    // TODO
  } else {
    addr -= 0x8000;
    prg_bank[addr / 0x4000][addr & 0x3fff] = value;
  }
}



// PPU-space access
//
//
void ROM::write_chr(uint8_t value, uint16_t addr) {
  chr_bank[addr / 0x1000][addr % 0x1000] = value;
}

uint8_t ROM::read_chr(uint16_t addr) const {
  return chr_bank[addr / 0x1000][addr % 0x1000];
}

void ROM::write_nt(uint8_t value, uint16_t addr) {
  uint16_t table = (addr >> 10) & 3;
  nametable[table][addr & 0x3ff] = value;
}

uint8_t ROM::read_nt(uint16_t addr) const {
  uint16_t table = (addr >> 10) & 3;
  return nametable[table][addr & 0x3ff];
}


ROM::ROM(const char *path):
  nt (0x800),
  nametable (4)
{

  std::ifstream file(path);

  if(!(
    file.get() == 'N' &&
    file.get() == 'E' &&
    file.get() == 'S' &&
    file.get() == 0x1A)){

    throw std::runtime_error ("Not iNES format");

  }

  uint8_t prg_rom_size = file.get();
  uint8_t chr_rom_size = file.get();
  uint8_t flag6 = file.get();
  uint8_t flag7 = file.get();

  // prg_ram_size = file.get();
  // flag9 = file.get();
  // flag10 = file.get();

  file.seekg(0x10);
  
  bool four_screen = flag6 & 4;
  bool horizontal_mirroring = !(flag6 & 1);

  int mapper_id { (flag6 >> 4) | (flag7 & 0xf0) };

  std::cout << "Mapper " << mapper_id << '\n';
  std::cout << "prg banks: " << (int)prg_rom_size << '\n';
  std::cout << "chr banks: " << (int)chr_rom_size << '\n';

  if (mapper_id != 0)
    throw std::runtime_error ("Unsupported mapper");

  prg.resize(0x4000 + prg_rom_size * 0x4000);
  chr.resize(0x2000 + chr_rom_size * 0x2000);

  file.read((char*)prg.data(), prg_rom_size * 0x4000);
  file.read((char*)chr.data(), chr_rom_size * 0x2000);

  if (four_screen) {
    // TODO
    std::cout << "Four screen\n";
    nametable[0] = nt.data();
    nametable[1] = nt.data();
    nametable[2] = nt.data();
    nametable[3] = nt.data();
  } else if (horizontal_mirroring) {
    std::cout << "Horizontal mirroring\n";
    nametable[0] = nt.data();
    nametable[1] = nt.data();
    nametable[2] = nt.data() + 0x400;
    nametable[3] = nt.data() + 0x400;
  } else {
    std::cout << "Vertical mirroring\n";
    nametable[0] = nt.data();
    nametable[1] = nt.data() + 0x400;
    nametable[2] = nt.data();
    nametable[3] = nt.data() + 0x400;
  }

  prg_bank.push_back(prg.data() + 0x0);
  prg_bank.push_back(prg.data() + (prg_rom_size > 1 ? 0x4000 : 0));

  chr_bank.push_back(chr.data());
  chr_bank.push_back(chr.data() + 0x1000);

}


ROM::~ROM() {}
