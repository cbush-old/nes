#ifndef MAPPERS_H
#define MAPPERS_H

#include "rom.h"
#include "bit.h"

#include "mmc3.h"

// Mapper 0
using NROM = ROM;


// Mapper 1
class SxROM : public ROM {
  public:
    SxROM();
    ~SxROM();

  public:
    virtual void write_prg(uint8_t value, uint16_t addr);

  public:
    virtual void set_prg(uint8_t);
    virtual void set_chr(uint8_t);

  protected:
    void regw(uint8_t value, uint16_t addr);

  private:
    uint8_t _register { 0 };
    uint8_t _write { 0 };
    union {
      bit<0, 8> _reg8;
      bit<0, 2> _mirror_mode;
      bit<2, 1> _slot_select;
      bit<3, 1> _prg_size;
      bit<4, 1> _chr_mode;

      bit<8, 8> _regA;
      bit<16, 8> _regC;

      bit<24, 5> _regE;
      bit<24, 4> _prg_reg;
      bit<28, 1> _wram_disabled;
    };
};

// Mapper 2
class UxROM : public ROM {
  public:
    UxROM(){}
    ~UxROM(){}

  public:
    virtual void write_prg(uint8_t value, uint16_t addr) override;

};

// Mapper 3
class CNROM : public ROM {
  public:
    CNROM(){}
    ~CNROM(){}

  public:
    virtual void write_prg(uint8_t value, uint16_t addr) override;

};


// Mapper 71
using Camerica = UxROM;

#endif