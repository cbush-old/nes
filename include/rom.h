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
    uint8_t read(uint16_t) const;
    uint8_t& getmemref(uint16_t);
    uint8_t& getvbankref(uint16_t addr);
    uint8_t& getntref(uint8_t, uint16_t);
    void write(uint8_t value, uint16_t addr);

  protected:
    static const unsigned VROM_Granularity = 0x400, VROM_Pages = 0x2000 / VROM_Granularity;
    static const unsigned ROM_Granularity = 0x2000, ROM_Pages = 0x10000 / ROM_Granularity;

  protected:
    // Real memory
    std::vector<uint8_t> _nram;
    std::vector<uint8_t> _rom;
    std::vector<uint8_t> _vram;
    std::vector<uint8_t> _pram;

    // Mirrors
    std::vector<uint8_t*> _nt;
    std::vector<uint8_t*> _bank;
    std::vector<uint8_t*> _vbank;

    std::function<void(ROM&, uint8_t, uint16_t)> writef;

};

#endif
