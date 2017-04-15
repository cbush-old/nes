#pragma once

#include "generator.h"

class IBus;

class DMC : public Generator
{
public:
    DMC(IBus &bus);
    void update();
    
private:
    virtual void enable() override;
    virtual void disable() override;
    virtual void reg3_write(uint8_t value) override;

    IBus &_bus;
    uint8_t _sample{ 0 };
    uint16_t _address{ 0 };
    uint16_t _bytes_remaining{ 0 };
    uint8_t _bits_remaining{ 1 };
    uint8_t _shift{ 0 };
    uint16_t _t{ 0 };
    bool _interrupt{ false };
    bool _silence{ false };
};
