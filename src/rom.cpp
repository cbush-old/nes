#include "rom.h"

#include "bit.h"
#include "clone_ptr.h"
#include "mappers/mappers.h"

#include <iostream>
#include <fstream>

// CPU-space access
//
//
uint8_t ROM::read_prg(uint16_t addr) const
{
    if (addr < 0x8000)
    {
        return 0;
        //return ram[addr % 0x4000];
    }
    else
    {
        addr -= 0x8000;
        return mirrored_prg(addr, 0x4000);
    }
}

void ROM::write_prg(uint8_t value, uint16_t addr)
{
    if (addr < 0x8000)
    {
        // ram[addr % 0x4000] = value;
    }
    else
    {
        assert(false);
        //addr -= 0x8000;
        //mirrored_prg(addr, 0x4000) = value;
    }
}

// PPU-space access
//
//
void ROM::write_chr(uint8_t value, uint16_t addr)
{
    assert(false); //mirrored_chr(addr, 0x1000) = value;
}

uint8_t ROM::read_chr(uint16_t addr) const
{
    return mirrored_chr(addr, 0x1000);
}

void ROM::write_nt(uint8_t value, uint16_t addr)
{
    mirrored_nt(addr, 0x400) = value;
}

uint8_t ROM::read_nt(uint16_t addr) const
{
    return mirrored_nt(addr, 0x400);
}

ROM::ROM()
{
    // std::ifstream file("save.hex");
    // file.read(reinterpret_cast<char *>(ram.data()), ram.size());
}

size_t ROM::get_prg_size_for_count(uint8_t count) const
{
    return count * 0x4000;
}

size_t ROM::get_chr_size_for_count(uint8_t count) const
{
    return count ? count * 0x2000 : 0x4000;
}

void ROM::set_prg(uint8_t count, RomRef rom)
{
    _prg = rom;
    _prg_bank[0] = 0;
    _prg_bank[1] = count > 1 ? 0x4000 : 0;
    on_set_prg(count);
}

void ROM::set_chr(uint8_t count, RomRef rom)
{
    _chr = rom;
    _chr_bank[0] = 0;
    _chr_bank[1] = 0x1000;
    on_set_chr(count);
}

void ROM::set_mirroring(MirrorMode mode)
{
    if (mode == FOUR_SCREEN)
    {
        std::cout << "4s mirroring\n";
        _nametable[0] = 0;
        _nametable[1] = 0;
        _nametable[2] = 0x400;
        _nametable[3] = 0x400;
    }
    else if (mode == HORIZONTAL)
    {
        // std::cout << "Horizontal mirroring\n";
        _nametable[0] = 0;
        _nametable[1] = 0;
        _nametable[2] = 0x400;
        _nametable[3] = 0x400;
    }
    else if (mode == VERTICAL)
    {
        // std::cout << "Vertical mirroring\n";
        _nametable[0] = 0;
        _nametable[1] = 0x400;
        _nametable[2] = 0;
        _nametable[3] = 0x400;
    }
    else if (mode == SINGLE_SCREEN_A)
    {
        std::cout << "1sa mirroring\n";
        _nametable[0] = 0;
        _nametable[1] = 0;
        _nametable[2] = 0;
        _nametable[3] = 0;
    }
    else if (mode == SINGLE_SCREEN_B)
    {
        std::cout << "1sb mirroring\n";
        _nametable[0] = 0x400;
        _nametable[1] = 0x400;
        _nametable[2] = 0x400;
        _nametable[3] = 0x400;
    }
}

ROM::~ROM()
{
    // std::ofstream file("save.hex");
    // file.write(reinterpret_cast<const char *>(ram.data()), ram.size());
}

void ROM::on_set_prg(uint8_t)
{}

void ROM::on_set_chr(uint8_t)
{}

template<typename Mirror>
uint8_t const &ROM::mirrored(RomRef const &source, Mirror const &mirror, uint16_t addr, uint16_t mod) const
{
    return source->operator[](mirror[addr / mod] + (addr % mod));
}

uint8_t const &ROM::mirrored_chr(uint16_t addr, uint16_t mod) const
{
    return mirrored(_chr, _chr_bank, addr, mod);
}

uint8_t const &ROM::mirrored_prg(uint16_t addr, uint16_t mod) const
{
    return mirrored(_prg, _prg_bank, addr, mod);
}

uint8_t const &ROM::mirrored_nt(uint16_t addr, uint16_t mod) const
{
    const auto i = (addr >> 10) & 3;
    return _nt[_nametable[i] + (addr % mod)];
}

uint8_t &ROM::mirrored_nt(uint16_t addr, uint16_t mod)
{
    return const_cast<uint8_t &>(static_cast<ROM const *>(this)->mirrored_nt(addr, mod));
}

InlinePolymorph<ROM> load_ROM(IBus *bus, const char *path)
{
    std::ifstream file(path);

    if (!(
            file.get() == 'N' &&
            file.get() == 'E' &&
            file.get() == 'S' &&
            file.get() == 0x1A))
    {

        throw std::runtime_error("Not iNES format");
    }

    uint8_t prg_rom_size = file.get();
    uint8_t chr_rom_size = file.get();
    uint8_t flag6 = file.get();
    uint8_t flag7 = file.get();

    // int prg_ram_size = file.get();
    // flag9 = file.get();
    // flag10 = file.get();

    file.seekg(0x10);

    bool four_screen = flag6 & 4;
    bool horizontal_mirroring = !(flag6 & 1);

    int mapper_id{ (flag6 >> 4) | (flag7 & 0xf0) };

    std::cout << "Mapper " << mapper_id << '\n';
    std::cout << "_prg banks: " << (int)prg_rom_size << '\n';
    std::cout << "_chr banks: " << (int)chr_rom_size << '\n';

    InlinePolymorph<ROM> rom;
    switch (mapper_id)
    {
    case 0:
        rom.emplace<NROM>();
        break;
    case 1:
        rom.emplace<SxROM>();
        break;
    case 2:
        rom.emplace<UxROM>();
        break;
    case 3:
        rom.emplace<CNROM>();
        break;
    case 4:
        rom.emplace<MMC3>(bus);
        break;
    case 71:
        rom.emplace<Camerica>();
        break;
    default:
        throw std::runtime_error("Unsupported mapper");
    }

    rom->set_mirroring(four_screen ? ROM::MirrorMode::FOUR_SCREEN : horizontal_mirroring ? ROM::MirrorMode::HORIZONTAL : ROM::MirrorMode::VERTICAL);
    // rom->write_prg(0xc, 0x8000); // fixme: move "set_prg" stuff to constructor?

    std::shared_ptr<std::vector<uint8_t>> _prg(new std::vector<uint8_t>(rom->get_prg_size_for_count(prg_rom_size)));
    std::shared_ptr<std::vector<uint8_t>> _chr(new std::vector<uint8_t>(rom->get_chr_size_for_count(chr_rom_size)));

    file.read(reinterpret_cast<char *>(_prg->data()), _prg->size());
    file.read(reinterpret_cast<char *>(_chr->data()), _chr->size());

    rom->set_prg(prg_rom_size, _prg);
    rom->set_chr(chr_rom_size, _chr);

    return rom;
}
