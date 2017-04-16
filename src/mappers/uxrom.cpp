#include "mappers.h"

#include <iostream>

void UxROM::write_prg(uint8_t value, uint16_t addr)
{
    if (addr < 0x8000)
    {
        return;
    }

    _prg_bank[0] = (value & 0xf) * 0x4000;
    _prg_bank[1] = _prg->size() - 0x4000;
}
