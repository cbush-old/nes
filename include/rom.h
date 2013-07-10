#ifndef ROM_H
#define ROM_H

#include <iostream>
#include <thread>
#include <cmath>
#include <ctime>
#include <map>
#include <iomanip>
#include <functional>
#include <cctype>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>

struct Mapperf;

struct ROM {
  
  friend class PPU;
  friend class CPU;
  
  static const unsigned VROM_Granularity = 0x400, VROM_Pages = 0x2000 / VROM_Granularity;
  static const unsigned ROM_Granularity = 0x2000, ROM_Pages = 0x10000 / ROM_Granularity;

  std::vector<uint8_t> nram, rom, vram, pram;
  std::vector<uint8_t*> nt, bank, vbank;
  std::function<void(ROM&, uint8_t, uint16_t)> writef;

  protected:
    uint8_t& operator[](uint16_t);
    inline uint8_t& vbank_ref(uint16_t addr){
      return vbank[(addr/VROM_Granularity)%VROM_Pages][addr%VROM_Granularity];
    }

    inline void write(uint8_t value, uint16_t addr){
      if(0x8000 <= addr)
        writef(*this, value, addr);
    }

  public:
    ROM(std::string const&);

};

#endif
