#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <thread>
#include <cmath>
#include <ctime>
#include <map>
#include <iomanip>
#include <functional>
#include <cctype>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_opengl.h>

#include "bus.h"
#include "rom.h"
#include "ppu.h"
#include "apu.h"
#include "io.h"

class CPU {
  private:
    enum Flag {
      N_FLAG = 0x80, V_FLAG = 0x40, D_FLAG = 0x08,
      I_FLAG = 0x04, Z_FLAG = 0x02, C_FLAG = 0x01
    };
    
    std::vector<uint8_t> memory;
    
    uint8_t 
      P { 0x24 },
      A { 0 },
      X { 0 },
      Y { 0 },
      SP { 0xff };
    uint16_t PC, cyc { 0 };
    int result_cycle { 0 };
    
  private:
    typedef void(CPU::*op)(); // operation
    typedef uint16_t(CPU::*mode)(); // addressing mode
    typedef uint8_t(CPU::*regr)(); // register read
    typedef void(CPU::*regw)(uint8_t); // register write
    typedef uint8_t(CPU::*condition)(); // branch condition
      
    template<Flag F> inline void set_if(bool cond){
      if(cond) P|=F; else P&=~F;
    }
  
    template<Flag F> inline void clear_if(bool cond){
      if(cond) P&=~F;
    }
  
    inline void setZN(uint8_t x){
      set_if<Z_FLAG>(!x);
      set_if<N_FLAG>(x&0x80);
    }
    
    uint8_t read(uint16_t);
    uint8_t write(uint8_t, uint16_t);
    void push(uint8_t);
    void push2(uint16_t);
    uint8_t pull();
    uint16_t pull2();
    uint8_t next();
    uint16_t next2();
    
    void addcyc();    
    
    template<mode M>
    uint8_t& getref(){
      uint16_t addr = (this->*M)();
      if(addr < 0x2000) return memory[addr & 0x7ff];
      if(addr < 0x4020){
        // this shouldn't happen
        return memory[0];
      }
      return bus::rom()[addr];
    }

    template<typename T> 
    inline uint16_t sum_check_pgx(uint16_t addr, T x){
      uint16_t r = addr + x;
      if((r&0xff00) != (addr&0xff00))
        addcyc();
      return r;
    }

  public:
    CPU();
    void pull_NMI();
    void run();
    void load_state(State const&);
    void save_state(State&) const;
    int test_cyc { 0 };
    
  private:
    inline void tick(){
      ++test_cyc;
    
    }
    
    // addressing modes
    inline uint16_t ACC(){ tick(); return 0; } // template only
    inline uint16_t X__(){ tick(); return 0; }
    inline uint16_t Y__(){ tick(); return 0; }
    inline uint16_t IMM(){ return next(); }
    inline uint16_t ZPG(){ tick(); return next(); }
    inline uint16_t ZPX(){ tick(); return (next() + X)&0xff; }
    inline uint16_t ZPY(){ tick(); return (next() + Y)&0xff; }
    inline uint16_t ABS(){ tick(); tick(); return next2(); }
    inline uint16_t ABX(){ tick(); return next2() + X; }
    inline uint16_t ABY(){ tick(); return next2() + Y; }
    inline uint16_t IDX(){
      tick(); 
      uint16_t addr = next() + X;
      return read(addr&0xff)|((uint16_t)read((addr+1)&0xff) << 8);
    }
    inline uint16_t IDY(){
      tick(); 
      uint16_t addr = next();
      return ((uint16_t)read(addr)|(read((addr+1)&0xff)<<8))+Y;
    }
    inline uint16_t IND(){
      tick(); 
      // When on page boundary (i.e. $xxFF) IND gets LSB from $xxFF like normal 
      // but takes MSB from $xx00
      uint16_t addr = next2();
      return addr&0xff == 0xff?
        read(addr)|(read(addr&0xff00) << 8) :
        read(addr)|(read(addr+1) << 8);
    }
    
    // some modes add cycles if page crossed
    inline uint16_t ABX_pgx(){ tick(); return sum_check_pgx(next2(), X); }
    inline uint16_t ABY_pgx(){ tick(); return sum_check_pgx(next2(), Y); }
    inline uint16_t IDY_pgx(){
      tick(); 
      uint16_t addr { next() };
      return sum_check_pgx((uint16_t)read(addr)|(read((addr + 1)&0xff) << 8), Y);
    }
    
    // register read/write
    inline void Accumulator(uint8_t i){ tick(); setZN(A = i); }
    inline void IndexRegX(uint8_t i){ tick(); setZN(X = i); }
    inline void IndexRegY(uint8_t i){ tick(); setZN(Y = i); }
    inline void ProcStatus(uint8_t i){ tick(); P = (i|0x20)&~0x10; }
    inline void StackPointer(uint8_t i){ tick(); SP = i; }
    inline uint8_t Accumulator(){ tick(); return A; }
    inline uint8_t IndexRegX(){ tick(); return X; }
    inline uint8_t IndexRegY(){ tick(); return Y; }
    inline uint8_t ProcStatus(){ tick(); return P|0x10; }
    inline uint8_t StackPointer(){ tick(); return SP; }
    inline uint8_t AX(){ tick(); return A&X; } // unofficial
    
    #include "cpu-ops.cc"

    static const op ops[256];
    static const char* const opasm[256];

    template<mode M> uint8_t read(){
      return read((this->*M)());
    }

};

template<> uint8_t& CPU::getref<&CPU::ACC>();
template<> uint8_t& CPU::getref<&CPU::X__>();
template<> uint8_t& CPU::getref<&CPU::Y__>();
template<> uint8_t CPU::read<&CPU::IMM>();

#endif
