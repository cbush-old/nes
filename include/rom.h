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

#include "bus.h"

class Mapperf;

class ROM : public IROM {
  public:
    ROM(std::string const&);

  public:
    uint8_t& getmemref(uint16_t);

  public:
    void write_nt(uint8_t value, uint16_t addr);
    void write_prg(uint8_t value, uint16_t addr);
    void write_chr(uint8_t value, uint16_t addr);
    uint8_t read_prg(uint16_t addr) const;
    uint8_t read_chr(uint16_t addr) const;
    uint8_t read_nt(uint16_t addr) const;

  protected:
    // Real memory
    std::vector<uint8_t> nt;
    std::vector<uint8_t> prg;
    std::vector<uint8_t> chr;

    // Mirrors
    std::vector<uint8_t*> nametable;
    std::vector<uint8_t*> prg_bank;
    std::vector<uint8_t*> chr_bank;

    std::function<void(ROM&, uint8_t, uint16_t)> writef;

  private:
    int
        prg_rom_size,
        chr_rom_size,
        flag6,
        flag7,
        prg_ram_size,
        flag9,
flag10;

};

#endif
