#ifndef CPU_H
#define CPU_H

#include "bus.h"

#include <array>
#include <cctype>
#include <cmath>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

class CPU
    : public ICPU
{
public:
    CPU(IBus *bus, IController *controller0, IController *controller1);
    
    CPU(CPU const &) = default;
    CPU(CPU &&) = default;
    CPU &operator=(CPU const &) = default;
    CPU &operator=(CPU &&) = default;
    virtual ~CPU() = default;

    virtual uint8_t read(uint16_t) override;

    virtual void pull_NMI() override;

    virtual void pull_IRQ() override;

    virtual void release_IRQ() override;

    virtual void update(double rate) override;

private:
    typedef void (CPU::*op)();               // operation
    typedef uint16_t (CPU::*mode)();         // addressing mode
    typedef uint8_t (CPU::*regr)();          // register read
    typedef void (CPU::*regw)(uint8_t);      // register write
    typedef uint8_t (CPU::*condition)();     // branch condition
    typedef uint8_t (CPU::*rmw_op)(uint8_t); // read-modify-write

    enum Flag
    {
        N_FLAG = 0x80,
        V_FLAG = 0x40,
        D_FLAG = 0x08,
        I_FLAG = 0x04,
        Z_FLAG = 0x02,
        C_FLAG = 0x01
    };

    template <Flag F>
    inline void set_if(bool cond)
    {
        if (cond)
            _P |= F;
        else
            _P &= ~F;
    }

    inline void setZN(uint8_t x)
    {
        set_if<Z_FLAG>(!x);
        set_if<N_FLAG>(x & 0x80);
    }

    void write(uint8_t, uint16_t);
    void push(uint8_t);
    void push2(uint16_t);
    uint8_t pull();
    uint16_t pull2();
    uint8_t next();
    uint16_t next2();

    void addcyc();

    // addressing modes
    inline uint16_t ACC() { return 0; } // template only
    inline uint16_t X__() { return 0; }
    inline uint16_t Y__() { return 0; }
    inline uint16_t IMM() { return next(); }
    inline uint16_t ZPG() { return next(); }
    inline uint16_t ZPX() { return (next() + _X) & 0xff; }
    inline uint16_t ZPY() { return (next() + _Y) & 0xff; }
    inline uint16_t ABS() { return next2(); }
    inline uint16_t ABX() { return next2() + _X; }
    inline uint16_t ABY() { return next2() + _Y; }
    inline uint16_t IDX()
    {
        uint16_t addr = next() + _X;
        return read(addr & 0xff) | ((uint16_t)read((addr + 1) & 0xff) << 8);
    }

    inline uint16_t IDY()
    {
        uint16_t addr = next();
        return ((uint16_t)read(addr) | (read((addr + 1) & 0xff) << 8)) + _Y;
    }

    inline uint16_t IND()
    {
        // When on page boundary (i.e. $xxFF) IND gets LSB from $xxFF like normal
        // but takes MSB from $xx00
        uint16_t addr = next2();
        return (addr & 0xff) == 0xff ?
                   read(addr) | (read(addr & 0xff00) << 8) :
                   read(addr) | (read(addr + 1) << 8);
    }

    // some modes add cycles if page crossed
    inline uint16_t ABX_pgx() { return sum_check_pgx(next2(), _X); }
    inline uint16_t ABY_pgx() { return sum_check_pgx(next2(), _Y); }
    inline uint16_t IDY_pgx()
    {

        uint16_t addr{ next() };
        return sum_check_pgx((uint16_t)read(addr) | (read((addr + 1) & 0xff) << 8), _Y);
    }

    // register read/write
    inline void Accumulator(uint8_t i) { setZN(_A = i); }
    inline void IndexRegX(uint8_t i) { setZN(_X = i); }
    inline void IndexRegY(uint8_t i) { setZN(_Y = i); }
    inline void ProcStatus(uint8_t i) { _P = (i | 0x20) & ~0x10; }
    inline void StackPointer(uint8_t i) { _SP = i; }
    inline uint8_t Accumulator() { return _A; }
    inline uint8_t IndexRegX() { return _X; }
    inline uint8_t IndexRegY() { return _Y; }
    inline uint8_t ProcStatus() { return _P | 0x10; }
    inline uint8_t StackPointer() { return _SP; }
    inline uint8_t AX() { return _A & _X; } // unofficial

    template <mode M>
    uint8_t read()
    {
        return read((this->*M)());
    }

#include "cpu-ops.cc"

    template <typename T>
    inline uint16_t sum_check_pgx(uint16_t addr, T const &x)
    {
        uint16_t r = addr + x;
        if ((r & 0xff00) != (addr & 0xff00))
            addcyc();
        return r;
    }

    void print_status() const;
    void dump_memory() const;

    static const op s_ops[256];
    static const char *const s_opasm[256];

    IBus *_bus;
    IController *_controller[2];

    std::array<uint8_t, 0x800> _memory;

    uint8_t
        _P{0x34},
        _A{0},
        _X{0},
        _Y{0},
        _SP{0xfd};
    uint16_t _PC{0xC000};
    uint16_t _cyc{0};
    uint16_t _last_PC;
    uint8_t _last_op;
    uint8_t _last_op_cycles{0};
    int _result_cycle{0};
    bool _irq{ false };
};

template <>
uint8_t CPU::read<&CPU::IMM>();

#endif
