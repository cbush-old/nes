#include "rom.h"

#include <fstream>

using std::ifstream;
using std::cout;
using std::setw;
using std::hex;
using std::vector;
using std::thread;
using std::string;
using std::exception;
using std::runtime_error;

uint8_t& ROM::operator[](uint16_t addr){
  return mem.at(addr >= 0x8000 ? addr - 0x6000 : addr);
}

void ROM::write(uint8_t value, uint16_t addr){
  mem[addr] = value;
}

ROM::ROM(){}
ROM::ROM(std::string const& src){
  
  ifstream file(src);
  
  if(!(
    file.get() == 'N' &&
    file.get() == 'E' &&
    file.get() == 'S' &&
    file.get() == 0x1A)){
    
    throw runtime_error ("Not iNES format");
    
  }
  
  int
    prg_rom_size { file.get() },
    chr_rom_size { file.get() },
    flag6 { file.get() },
    flag7 { file.get() },
    prg_ram_size { file.get() },
    flag9 { file.get() },
    flag10 { file.get() };
    
  file.seekg(0x10);
  
  int mapper_id { (flag6 >> 4) | (flag7 & 0xf0) };

  switch(mapper_id){
    case 0:
      mem.resize(0xa000);
      file.read((char*)mem.data() + 0x2000, 0x4000);
      if(prg_rom_size == 1){
        file.seekg(0x10); // Loop data
      }
      
      file.read((char*)mem.data() + 0x6000, 0x4000);
      file.read((char*)mem.data(), 0x2000);
      
      break;
    default:
      throw runtime_error ("Unsupported mapper");
  
  }

}

