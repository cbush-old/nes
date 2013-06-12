#ifndef BUS_H
#define BUS_H

#include <string>

class ROM;
class CPU;
class PPU;
class APU;
class IO;

namespace bus {
  
  CPU& cpu();
  ROM& rom();
  PPU& ppu();
  APU& apu();
  IO& io();
  void pull_NMI();
  void play(std::string const&);
  
}

#endif
