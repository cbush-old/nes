#ifndef ROM_H
#define ROM_H

#include "bus.h"
#include "inline_polymorph.h"

#include <memory>

class ROM : public IROM
{
public:
    enum MirrorMode
    {
        HORIZONTAL,
        VERTICAL,
        FOUR_SCREEN,
        SINGLE_SCREEN_A,
        SINGLE_SCREEN_B,
    };

    using RomRef = std::shared_ptr<const std::vector<uint8_t>>;

    ROM();
    ROM &operator=(ROM const &) = default;
    ROM &operator=(ROM &&) = default;
    ROM(ROM const &) = default;
    ROM(ROM &&) = default;
    virtual ~ROM();

    virtual void write_nt(uint8_t value, uint16_t addr) override;
    virtual void write_prg(uint8_t value, uint16_t addr) override;
    virtual void write_chr(uint8_t value, uint16_t addr) override;
    virtual uint8_t read_prg(uint16_t addr) const override;
    virtual uint8_t read_chr(uint16_t addr) const override;
    virtual uint8_t read_nt(uint16_t addr) const override;

    void set_prg(uint8_t count, RomRef rom);
    void set_chr(uint8_t count, RomRef rom);

    virtual size_t get_prg_size_for_count(uint8_t count) const;
    virtual size_t get_chr_size_for_count(uint8_t count) const;

    virtual void set_mirroring(MirrorMode);

protected:
    uint8_t const &mirrored_chr(uint16_t addr, uint16_t mod) const;
    uint8_t const &mirrored_prg(uint16_t addr, uint16_t mod) const;
    uint8_t const &mirrored_nt(uint16_t addr, uint16_t mod) const;
    uint8_t &mirrored_nt(uint16_t addr, uint16_t mod);

    // Real memory
    std::array<uint8_t, 0x800> _nt; // this is partially on physical PPU
    RomRef _prg;
    RomRef _chr;

    // Mirrors (indices)
    std::array<size_t, 4> _nametable;
    std::array<size_t, 8> _prg_bank;
    std::array<size_t, 8> _chr_bank;
    
private:
    virtual void on_set_prg(uint8_t);
    virtual void on_set_chr(uint8_t);

    template<typename Mirror>
    uint8_t const &mirrored(RomRef const &source, Mirror const &mirror, uint16_t addr, uint16_t mod) const;

};

InlinePolymorph<ROM> load_ROM(IBus *, const char *);

#endif
