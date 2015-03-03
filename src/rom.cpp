#include "rom.h"
#include "bit.h"

#include "mappers/mappers.h"

#include <iostream>
#include <fstream>

// CPU-space access
//
//
uint8_t ROM::read_prg(uint16_t addr) const {
  if (addr < 0x8000) {
    return ram[addr % 0x4000];
  } else {
    addr -= 0x8000;
    return prg_bank[addr / 0x4000][addr & 0x3fff];
  }
}

void ROM::write_prg(uint8_t value, uint16_t addr) {
  if (addr < 0x8000) {
    ram[addr % 0x4000] = value;
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


ROM::ROM()
  : nt (0x800)
  , ram (0x4000)
  , nametable (4)
{
  std::ifstream file ("save.hex");
  file.read(reinterpret_cast<char*>(ram.data()), ram.size());
}

void ROM::set_prg(uint8_t count) {

  prg.resize(count * 0x4000);

  prg_bank.push_back(prg.data() + 0x0);
  prg_bank.push_back(prg.data() + (count > 1 ? 0x4000 : 0));

}

void ROM::set_chr(uint8_t count) {

  chr.resize(count ? count * 0x2000 : 0x4000);

  chr_bank.push_back(chr.data());
  chr_bank.push_back(chr.data() + 0x1000);

}

uint8_t* ROM::get_prg_data() {
  return prg.data();
}

uint8_t* ROM::get_chr_data() {
  return chr.data();
}

size_t ROM::get_prg_size() const {
  return prg.size();
}

size_t ROM::get_chr_size() const {
  return chr.size();
}

void ROM::set_mirroring(MirrorMode mode) {
  if (mode == FOUR_SCREEN) {
    std::cout << "4s mirroring\n";
    nametable[0] = nt.data();
    nametable[1] = nt.data();
    nametable[2] = nt.data() + 0x400;
    nametable[3] = nt.data() + 0x400;

  } else if (mode == HORIZONTAL) {
    // std::cout << "Horizontal mirroring\n";
    nametable[0] = nt.data();
    nametable[1] = nt.data();
    nametable[2] = nt.data() + 0x400;
    nametable[3] = nt.data() + 0x400;
  } else if (mode == VERTICAL) {
    // std::cout << "Vertical mirroring\n";
    nametable[0] = nt.data();
    nametable[1] = nt.data() + 0x400;
    nametable[2] = nt.data();
    nametable[3] = nt.data() + 0x400;
  } else if (mode == SINGLE_SCREEN_A) {
    std::cout << "1sa mirroring\n";
    nametable[0] = nt.data();
    nametable[1] = nt.data();
    nametable[2] = nt.data();
    nametable[3] = nt.data();
  } else if (mode == SINGLE_SCREEN_B) {
    std::cout << "1sb mirroring\n";
    nametable[0] = nt.data() + 0x400;
    nametable[1] = nt.data() + 0x400;
    nametable[2] = nt.data() + 0x400;
    nametable[3] = nt.data() + 0x400;
  }
}

ROM::~ROM() {
  std::ofstream file ("save.hex");
  file.write(reinterpret_cast<const char*>(ram.data()), ram.size());
}

IROM *load_ROM(IBus *bus, const char *path) {

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

  // int prg_ram_size = file.get();
  // flag9 = file.get();
  // flag10 = file.get();

  file.seekg(0x10);
  
  bool four_screen = flag6 & 4;
  bool horizontal_mirroring = !(flag6 & 1);

  int mapper_id { (flag6 >> 4) | (flag7 & 0xf0) };

  std::cout << "Mapper " << mapper_id << '\n';
  std::cout << "prg banks: " << (int)prg_rom_size << '\n';
  std::cout << "chr banks: " << (int)chr_rom_size << '\n';

  ROM *rom;
  switch (mapper_id) {
    case 0: rom = new NROM(); break;
    case 1: rom = new SxROM(); break;
    case 2: rom = new UxROM(); break;

    case 4: rom = new MMC3(bus); break;
    case 71: rom = new Camerica(); break;
    default:
      throw std::runtime_error ("Unsupported mapper");
  }

  rom->set_mirroring(
    four_screen ? ROM::MirrorMode::FOUR_SCREEN : horizontal_mirroring ? ROM::MirrorMode::HORIZONTAL : ROM::MirrorMode::VERTICAL
  );
  rom->set_prg(prg_rom_size);
  rom->set_chr(chr_rom_size);
  rom->write_prg(0xc, 0x8000); // fixme: move "set_prg" stuff to constructor

  file.read((char*)rom->get_prg_data(), rom->get_prg_size());
  file.read((char*)rom->get_chr_data(), rom->get_chr_size());

  return rom;

}


void unload_ROM(IROM *rom) {
  delete rom;
}
