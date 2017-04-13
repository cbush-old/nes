#ifndef MAPPER_MMC3_H
#define MAPPER_MMC3_H

#include "rom.h"
#include "bit.h"

#include <array>

// Mapper 4
class MMC3 : public ROM
{
public:
    MMC3(IBus *);
    ~MMC3();

public:
    virtual void write_prg(uint8_t value, uint16_t addr) override;
    virtual void write_chr(uint8_t value, uint16_t addr) override;
    virtual uint8_t read_chr(uint16_t addr) const override;
    virtual uint8_t read_prg(uint16_t addr) const override;

public:
    virtual void set_prg(uint8_t) override;
    virtual void set_chr(uint8_t) override;

protected:
    void setup();

protected:
    mutable IBus *_bus;
    std::array<uint8_t, 0x2000> _ram;

    union
    {
        bit<0, 8> _reg8000;
        bit<0, 3> _address;
        bit<6, 1> _prg_mode_1;
        bit<7, 1> _chr_mode_1;

        bit<8, 1> _mirroring;

        bit<24, 8> _regA001;
        bit<30, 1> _wram_write_protected;
        bit<31, 1> _wram_enabled;
    };

    std::array<uint8_t, 8> _reg8001;

    mutable uint8_t _IRQ_counter{ 0 };
    uint8_t _IRQ_reload{ 0 };
    bool _IRQ_enabled{ true };
    mutable bool _IRQ_pending{ false };
    mutable bool _edge{ false };
    mutable uint8_t _edge_counter{ 0 };
};

#endif
