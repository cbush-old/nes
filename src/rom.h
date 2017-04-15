#ifndef ROM_H
#define ROM_H

#include "bus.h"
#include "clone_ptr.h"

class ROM : public IROM
{
public:
    ROM();
    virtual ~ROM();

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
    virtual uint8_t *get_prg_data();
    virtual uint8_t *get_chr_data();
    virtual size_t get_prg_size() const;
    virtual size_t get_chr_size() const;

public:
    enum MirrorMode
    {
        HORIZONTAL,
        VERTICAL,
        FOUR_SCREEN,
        SINGLE_SCREEN_A,
        SINGLE_SCREEN_B,
    };
    virtual void set_mirroring(MirrorMode);

protected:
    uint8_t const &mirrored_chr(uint16_t addr, uint16_t mod) const;
    uint8_t &mirrored_chr(uint16_t addr, uint16_t mod);
    uint8_t const &mirrored_prg(uint16_t addr, uint16_t mod) const;
    uint8_t &mirrored_prg(uint16_t addr, uint16_t mod);
    uint8_t const &mirrored_nt(uint16_t addr, uint16_t mod) const;
    uint8_t &mirrored_nt(uint16_t addr, uint16_t mod);

    // Real memory
    std::vector<uint8_t> nt;
    std::vector<uint8_t> prg;
    std::vector<uint8_t> chr;
    std::vector<uint8_t> ram;

    // Mirrors (indices)
    std::vector<size_t> nametable;
    std::vector<size_t> prg_bank;
    std::vector<size_t> chr_bank;
    
private:
    uint8_t const &mirrored(std::vector<uint8_t> const &source, std::vector<size_t> const &mirror, uint16_t addr, uint16_t mod) const;

    uint8_t &mirrored(std::vector<uint8_t> &source, std::vector<size_t> const &mirror, uint16_t addr, uint16_t mod);

};

ClonePtr<IROM> load_ROM(IBus *, const char *);

#endif
