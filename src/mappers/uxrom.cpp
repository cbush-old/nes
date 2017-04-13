#include "mappers.h"

#include <iostream>

void UxROM::write_prg(uint8_t value, uint16_t addr)
{
    if (addr < 0x8000)
    {
        return;
    }

    prg_bank[0] = prg.data() + (value & 0xf) * 0x4000;
    prg_bank[1] = prg.data() + prg.size() - 0x4000;
}
