#include "cpu.h"
#include <thread>

using std::cout;
using std::setw;
using std::hex;
using std::vector;
using std::string;
using std::ifstream;
using std::exception;
using std::runtime_error;

// CPU cycle chart
static const uint8_t cycles[256]
{
    //////// 0 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
    /*0x00*/ 7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
    /*0x10*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*0x20*/ 6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
    /*0x30*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*0x40*/ 6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
    /*0x50*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*0x60*/ 6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
    /*0x70*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*0x80*/ 2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    /*0x90*/ 2, 6, 0, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
    /*0xA0*/ 2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    /*0xB0*/ 2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
    /*0xC0*/ 2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    /*0xD0*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*0xE0*/ 2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    /*0xF0*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
};

uint8_t CPU::read(uint16_t addr)
{

    if (addr < 0x2000)
    {

        return memory[addr & 0x7ff];
    }
    if (addr < 0x4000)
    {
        return bus->read_ppu(addr & 7);
    }
    else if (addr < 0x4020)
    {
        switch (addr & 0x1f)
        {
        case 0x15:
            return bus->read_apu();
        case 0x16:
            return controller[0]->read();
        case 0x17:
            return controller[1]->read();
        default:
            return 0;
        }
    }
    else
    {
        return bus->read_prg(addr);
    }
}

void CPU::write(uint8_t value, uint16_t addr)
{

    if (addr < 0x2000)
    { // Internal RAM

        // $0800 to $1fff: mirrors of $0000 - $0800
        memory[addr & 0x7ff] = value;
    }
    else if (addr < 0x4000)
    {
        bus->write_ppu(value, addr & 7);
    }
    else if (addr < 0x4020)
    {
        // APU and I/O registers
        switch (addr & 0x1f)
        {
        case 0x14:
        { // DMA transfer
            for (int i = 0; i < 256; ++i)
            {
                bus->write_ppu(read((value & 7) * 0x100 + i), 4 /* OAM_DATA */);
            }
        }
        break;
        case 0x16:
            controller[value & 1]->strobe();
            break;
        default:
            bus->write_apu(value, addr & 0x1f);
            break;
        }
    }
    else
    {
        bus->write_prg(value, addr);
    }
}

void CPU::push(uint8_t x)
{
    memory[0x100 + SP--] = x;
}

void CPU::push2(uint16_t x)
{
    memory[0x100 + SP--] = (uint8_t)(x >> 8);
    memory[0x100 + SP--] = (uint8_t)(x & 0xff);
}

uint8_t CPU::pull()
{
    return memory[++SP + 0x100];
}

uint16_t CPU::pull2()
{
    uint16_t r = memory[++SP + 0x100];
    return r | (memory[++SP + 0x100] << 8);
}

void CPU::addcyc()
{
    ++result_cycle;
}

uint8_t CPU::next()
{
    return read(PC++);
}

uint16_t CPU::next2()
{
    uint16_t v = (uint16_t)read(PC++);
    return v | ((uint16_t)read(PC++) << 8);
}

CPU::CPU(IBus *bus, IController *controller0, IController *controller1)
    : bus(bus)
    , controller
    {
        controller0,
        controller1,
    }
{
    memory[0x008] = 0xf7;
    memory[0x009] = 0xef;
    memory[0x00a] = 0xdf;
    memory[0x00f] = 0xbf;
    memory[0x1fc] = 0x69;

    PC = read(0xfffc) | (read(0xfffd) << 8);
    logi("PC: %x", (uint16_t)PC);
}

bool do_NMI{ false };

void CPU::pull_NMI()
{
    do_NMI = true;
}

void CPU::update(double rate)
{
    last_PC = PC;
    last_op = next();

#ifdef DEBUG_CPU
    print_status();
#endif

    (this->*ops[last_op])();

    for (int i = 0; i < cycles[last_op] + result_cycle; ++i)
    {
        bus->on_cpu_tick();
    }

    result_cycle = 0;

    if (IRQ && ((P & I_FLAG) == 0))
    {
        push2(PC);
        stack_push<&CPU::ProcStatus>();
        P |= I_FLAG;
        PC = read(0xfffe) | (read(0xffff) << 8);
    }
    else if (do_NMI)
    {
        do_NMI = false;
        push2(PC);
        stack_push<&CPU::ProcStatus>();
        PC = read(0xfffa) | (read(0xfffb) << 8);
    }
}

template <>
uint8_t CPU::read<&CPU::IMM>()
{
    return (uint8_t)IMM();
}

void CPU::print_status() const
{
    cout
        << hex << std::uppercase << std::setfill('0')
        << setw(4) << last_PC << "  "
        << setw(2) << (int)last_op << "   "
        << std::setfill(' ') << setw(16)
        << std::left << opasm[last_op]
        << std::setfill('0')
        << " A:" << setw(2) << (int)A
        << " X:" << setw(2) << (int)X
        << " Y:" << setw(2) << (int)Y
        << " P:" << setw(2) << (int)P
        << " SP:" << setw(2) << (int)SP
        << std::setfill(' ')
        << " CYC:" << setw(3) << std::dec << (int)cyc
        //<< " SL:" << setw(3) << (int)bus::debug_ppu_get_scanline()
        << hex << std::setfill('0')
        << " ST0:" << setw(2) << (int)memory[0x101 + SP]
        << " ST1:" << setw(2) << (int)memory[0x102 + SP]
        << " ST2:" << setw(2) << (int)memory[0x103 + SP]
        << '\n';
}

void CPU::pull_IRQ()
{
    IRQ = true;
}

void CPU::release_IRQ()
{
    IRQ = false;
}

void CPU::dump_memory() const
{
    int w = 16;
    std::printf("CPU   ");
    for (int i = 0; i < w; ++i)
    {
        std::printf("%02x ", i);
    }
    for (int i = 0; i < 0x800; ++i)
    {
        if (i % w == 0)
        {
            std::printf("\n%03x   ", i);
        }
        std::printf("%02x ", (uint8_t)memory[i]);
    }
    std::printf("\n");
}
