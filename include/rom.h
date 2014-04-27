#ifndef ROM_H
#define ROM_H

#include "bus.h"


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
    std::array<uint8_t, 0x800> nt;
    std::vector<uint8_t> prg;
    std::vector<uint8_t> chr;

    // Mirrors
    std::array<uint8_t*, 4> nametable;
    std::vector<uint8_t*> prg_bank;
    std::vector<uint8_t*> chr_bank;

};

#endif
