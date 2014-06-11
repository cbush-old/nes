#ifndef ROM_H
#define ROM_H

#include "bus.h"

class ROM : public IROM {
  public:
    ROM();
    virtual ~ROM();

  public:
    virtual uint8_t& getmemref(uint16_t);

  public:
    virtual void write_nt(uint8_t value, uint16_t addr);
    virtual void write_prg(uint8_t value, uint16_t addr);
    virtual void write_chr(uint8_t value, uint16_t addr);
    virtual uint8_t read_prg(uint16_t addr) const;
    virtual uint8_t read_chr(uint16_t addr) const;
    virtual uint8_t read_nt(uint16_t addr) const;

  public:
    virtual void set_prg(uint8_t);
    virtual void set_chr(uint8_t);
    virtual void reset();
    virtual uint8_t *get_prg_data();
    virtual uint8_t *get_chr_data();

  public:
    enum MirrorMode {
        HORIZONTAL,
        VERTICAL,
        FOUR_SCREEN,
    };
    virtual void set_mirroring(MirrorMode);

  protected:
    // Real memory
    std::vector<uint8_t> nt;
    std::vector<uint8_t> prg;
    std::vector<uint8_t> chr;

    // Mirrors
    std::vector<uint8_t*> nametable;
    std::vector<uint8_t*> prg_bank;
    std::vector<uint8_t*> chr_bank;

};

ROM *load_ROM(const char *);

void unload_ROM(ROM*);

#endif
