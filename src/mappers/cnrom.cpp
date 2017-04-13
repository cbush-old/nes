#include "mappers.h"

void CNROM::write_prg(uint8_t value, uint16_t addr)
{
    if (addr < 0x8000)
    {
        return;
    }

    auto reg = value * 0x2000;
    chr_bank[0] = chr.data() + reg;
    chr_bank[1] = chr.data() + 0x1000 + reg;
}

