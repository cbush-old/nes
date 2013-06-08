#include <iostream>
#include <map>
#include <iomanip>
#include <functional>
#include <cctype>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_opengl.h>

using std::cout;
using std::setw;
using std::hex;
using std::vector;
using std::string;
using std::ifstream;
using std::exception;
using std::runtime_error;

class ROM;
class CPU;
class PPU;
class APU;
class IO;

namespace bus {
  CPU& cpu();
  ROM& rom();
  PPU& ppu();
  APU& apu();
  IO& io();
}

// CPU cycle chart
static const uint8_t cycles[256] {
      // 0 1 2 3 4 5 6 7 8 9 A B C D E F
/*0x00*/ 7,6,0,8,3,3,5,5,3,2,2,2,4,4,6,6,
/*0x10*/ 2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0x20*/ 6,6,0,8,3,3,5,5,4,2,2,2,4,4,6,6,
/*0x30*/ 2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0x40*/ 6,6,0,8,3,3,5,5,3,2,2,2,3,4,6,6,
/*0x50*/ 2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0x60*/ 6,6,0,8,3,3,5,5,4,2,2,2,5,4,6,6,
/*0x70*/ 2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0x80*/ 2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
/*0x90*/ 2,6,0,6,4,4,4,4,2,5,2,5,5,5,5,5,
/*0xA0*/ 2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
/*0xB0*/ 2,5,0,5,4,4,4,4,2,4,2,4,4,4,4,4,
/*0xC0*/ 2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
/*0xD0*/ 2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0xE0*/ 2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
/*0xF0*/ 2,5,0,8,4,4,6,6,2,4,2,7,4,4,7,7
};

class ROM {
  protected:
    std::vector<uint8_t> mem;

  public:
    uint8_t& operator[](uint16_t addr){
      return mem.at(addr >= 0x8000 ? addr - 0x6000 : addr);
    }
    
    void write(uint8_t value, uint16_t addr){
      mem[addr] = value;
    }

  public:
    ROM(){}
    ROM(std::string const& src){
      
      ifstream file(src);
      
      if(!(
        file.get() == 'N' &&
        file.get() == 'E' &&
        file.get() == 'S' &&
        file.get() == 0x1A)){
        
        throw runtime_error ("Not iNES format");
        
      }
      
      int
        prg_rom_size { file.get() },
        chr_rom_size { file.get() },
        flag6 { file.get() },
        flag7 { file.get() },
        prg_ram_size { file.get() },
        flag9 { file.get() },
        flag10 { file.get() };
        
      file.seekg(0x10);
      
      int mapper_id { (flag6 >> 4) | (flag7 & 0xf0) };

      switch(mapper_id){
        case 0:
          mem.resize(0xa000);
          file.read((char*)mem.data() + 0x2000, 0x4000);
          if(prg_rom_size == 1){
            file.seekg(0x10); // Loop data
          }
          
          file.read((char*)mem.data() + 0x6000, 0x4000);
          file.read((char*)mem.data(), 0x2000);
          
          break;
        default:
          throw runtime_error ("Unsupported mapper");
      
      }
    
    }
    
};


class IO {
  
  friend class CPU;
  friend class PPU;
  
  private:
    SDL_Window *window {
      SDL_CreateWindow(
        "",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
        512,480,SDL_WINDOW_OPENGL
      )
    };
    
    SDL_GLContext glcon {
      SDL_GL_CreateContext(window)
    };
    
  private:
    uint8_t handle_input(){
      SDL_Event e;
      while(SDL_PollEvent(&e)){
        switch(e.type){
          case SDL_KEYDOWN:
            switch(e.key.keysym.sym){
            
            }
            return 0;
          case SDL_QUIT:
            return 1;
        }
      }
    }
    
    void swap(){
      SDL_GL_SwapWindow(window);
    }
    
    void put_pixel(int x, int y, char r, char g, char b){
      glColor3b(r,g,b);
      glBegin(GL_QUADS);
      glVertex2i(x,y);
      glVertex2i(x+1,y);
      glVertex2i(x+1,y+1);
      glVertex2i(x,y+1);
      glEnd();
    }
    
  public:
    IO(){
      glMatrixMode(GL_PROJECTION|GL_MODELVIEW);
      glLoadIdentity();
      glOrtho(0,256,0,240,0,1);
      glClearColor(0.2,0.2,0.2,1);
      glClear(GL_COLOR_BUFFER_BIT);
      /*
      for(int i = 0; i < 240; ++i)
        for(int j = 0; j < 256; ++j)
          put_pixel(j, i, rand()%0xff, rand()%0xff, rand()%0xff);
      */ 
      swap();
    }
    
    IO(IO const&) = delete;
    ~IO(){
      SDL_GL_DeleteContext(glcon);
      SDL_DestroyWindow(window);
    }


};

class PPU {
  
  friend class CPU;
  
  private:
    std::vector<uint8_t> ram, plt, oam;    
    
    // registers
    uint8_t
      ctrl, mask, status { 0xa0 }, 
      oamaddr, oamdata,
      ppuscroll[2], ppuaddr[2], data;

    bool wr2;
    
    struct {
      uint8_t sprindex, y, attr, x;
      uint16_t pattern;
    } OAM2[8], OAM3[8];
    
    uint16_t SL { 261 },
      frame { 0 },
      cycle { 340 },
      SL_end;
    
    uint16_t bg_palette; // palette latch
    uint16_t bg_shiftreg16[2]; // bmp data for two tiles
    uint8_t bg_shiftreg8[2]; // palette attributes
    
    uint16_t loopy_v, loopy_t, loopy_x;
    uint8_t buffer2007;
    
    bool NMI_occurred, NMI_output;
  
    PPU& tick(){
      
      return *this;
    }
    
    inline bool toggle(bool& x){ return x ^= 1; }
    
    inline uint16_t nt_mirror(uint16_t addr){
      addr &= 0xfff;
      // horizontal?
      if(addr < 0x400) return addr;
      if(addr < 0x800) return addr - 0x400;
      if(addr < 0xc00) return addr - 0x400;
      return addr - 0x800;
    }
    
    uint8_t read(uint16_t addr){
      if(addr < 0x2000) return bus::rom()[addr];
      if(addr < 0x3f00) return ram[nt_mirror(addr)];
      return plt[addr&0x1f];
    }
    
    void write(uint8_t value, uint16_t addr){
      if(addr < 0x2000) bus::rom().write(value, addr);
      else if(addr < 0x3f00) ram[nt_mirror(addr)] = value;
      else plt[addr&0x1f] = value;
    }
    
    using regrf = std::function<uint8_t()>;
    using regwf = std::function<void(uint8_t)>;
    
    regrf bad_read {[]{ return 0; }};
    regwf bad_write {[](uint8_t){}};
    
    std::vector<regrf> regrfs {
      /* 0 */ bad_read, 
      /* 1 */ bad_read,
      /* 2 */ [&] {
        // account for race condition
        uint8_t res = (unsigned)(SL * 341 + cycle - 82180) < 2 ? 
          status & 0x7F : status|(NMI_occurred << 7);
        
        // clear the latch, NMI occurred and vblank flag
        wr2 = false;//latch?
        NMI_occurred = false;
        status = res & 0x7f;
        return res;
        
      },
      /* 3 */ bad_read,
      /* 4 */ [&] { return oam[oamaddr]; },
      /* 5 */ bad_read, 
      /* 6 */ bad_read,
      /* 7 */ [&] {
        if(loopy_v < 0x3f00){
          data = buffer2007;
          buffer2007 = read(loopy_v);
        } else {
          data = read(loopy_v);
        }
        loopy_v += bool(ctrl & 4) * 31 + 1;
        return data;
      }
    };
    
    std::vector<regwf> regwfs {
      /* 0 */ [&](uint8_t value){
        loopy_t = (loopy_t & 0x73ff)|((value&3) << 10);
        NMI_output = (bool)(value&0x80);
        ctrl = value;
      },
      
      /* 1 */ [&](uint8_t value){ mask = value; },
      /* 2 */ bad_write,
      /* 3 */ [&](uint8_t value){ oamaddr = value; },
      /* 4 */ [&](uint8_t value){ oam[oamaddr++] = value; },
      /* 5 */ [&](uint8_t value){
        if(toggle(wr2)){
          loopy_t = (loopy_t & 0x7fe0) | (value >> 3);
          loopy_x = value&7;
        } else {
          loopy_t = (loopy_t & 0x7c1f) | ((value & 0xf8) << 2);
          loopy_t = (loopy_t & 0x0fff) | ((value & 7) << 12);
        }
      },
      /* 6 */ [&](uint8_t value){
        if(toggle(wr2)){
          loopy_t = (loopy_t & 0xff) | ((value & 0x3f) << 8);
        } else {
          loopy_t = (loopy_t & 0x3f00)|value;
          loopy_v = loopy_t;
        }        
      },
      /* 7 */ [&](uint8_t value){
        write(value, loopy_v);
        loopy_v += bool(ctrl&4) * 31 + 1;
      },
    };
    
    uint8_t regr(uint8_t reg){
      return regrfs[reg&7]();
    }
    
    uint8_t regw(uint8_t value, uint8_t reg){
      regwfs[reg&7](value);
    }
    
    
  public:
    PPU(): ram(0x800), plt(0x20), oam(0x100) {}
  
  
};

class CPU {
  private:
    enum Flag {
      N_FLAG = 0x80,
      V_FLAG = 0x40,
      D_FLAG = 0x08,
      I_FLAG = 0x04,
      Z_FLAG = 0x02,
      C_FLAG = 0x01
    };
    
    vector<uint8_t> memory;
    uint8_t 
      P { 0x24 },
      A { 0 },
      X { 0 },
      Y { 0 },
      SP { 0xfd };
    uint16_t PC, cyc { 0 };
    
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
    
    uint8_t read(uint16_t addr){
      if(addr < 0x2000) return memory[addr&0x7ff];
      if(addr < 0x4000) return bus::ppu().regr(addr&7);
      if(addr < 0x4020){
        switch(addr&0x1f){
          case 0x15:
          case 0x16: // controller 1
          case 0x17: // controller 2
          default: return 0;
        }
      }
      
      return bus::rom()[addr];
      
    }
    
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
    
    uint8_t write(uint8_t value, uint16_t addr){
      if(addr < 0x2000) return memory[addr&0x7ff] = value;
      if(addr < 0x4000) return bus::ppu().regw(value, addr&7);
      if(addr < 0x4020){
        switch(addr&0x1f){
          case 0x14: {
            // DMA transfer
            PPU& ppu = bus::ppu();
            for(int i=0; i < 256; ++i){
              ppu.write(read((value&7)*0x100 + i), 0x4);
            }
          } break;
          
          default: return 0;
          
        }
      }
      
      return bus::rom()[addr] = value;
      
    }
    
    void push(uint8_t x){
      memory[0x100 + SP--] = x;
    }
    
    void push2(uint16_t x){
      memory[0x100 + SP--] = (uint8_t)(x >> 8);
      memory[0x100 + SP--] = (uint8_t)(x&0xff);
    }
  
    uint8_t pull(){
      return memory[++SP + 0x100];
    }
    
    uint16_t pull2(){
      uint16_t r = memory[++SP + 0x100];
      return r | memory[++SP + 0x100] << 8;    
    }
    
    void addcyc(){
      cyc += 3;
      bus::ppu().tick().tick().tick();
    }
    
    template<typename T> 
    inline uint16_t sum_check_pgx(uint16_t addr, T x){
      uint16_t r = addr + x;
      if((r&0xff00) != (addr&0xff00))
        addcyc();
      return r;
    }
    
    inline uint8_t next(){
      return read(PC++);
    }
    
    inline uint16_t next2(){
      uint16_t v = (uint16_t)read(PC++);
      return v | ((uint16_t)read(PC++) << 8);
    }
    
  public:
    CPU():memory(0x800, 0xff){
      memory[0x008] = 0xf7;
      memory[0x009] = 0xef;
      memory[0x00a] = 0xdf;
      memory[0x00f] = 0xbf;
      memory[0x1fc] = 0x69;
    }
    
    void pull_NMI(){
      push2(PC);
      push(P);
      PC = read(0xfffa) | (read(0xfffb) << 8);
    }
    
    void run(){
    
      PC = read(0xfffc) | (read(0xfffd) << 8);
      
      int SL = 0;
      
      for(;;){
    
        auto last_PC = PC;
        
        uint8_t last_op = next();
      
#ifdef DEBUG_CPU
        cout 
          << hex << std::uppercase << std::setfill('0')
          << setw(4) << last_PC << "  "
          << setw(2) << (int)last_op << "   "
          //<< opasm[last_op] << '\t'
          << " A:" << setw(2) << (int)A
          << " X:" << setw(2) << (int)X
          << " Y:" << setw(2) << (int)Y
          << " P:" << setw(2) << (int)P
          << " SP:" << setw(2) << (int)SP
          << std::setfill(' ')
          << " CYC:" << setw(3) << std::dec << (int)cyc
          //<< " SL:" << setw(2) << (int)SL
          << '\n';
#endif
        
        if(!last_op)
          break;
        
        (this->*ops[last_op])();
        
        for(int i = 0; i < cycles[last_op]; ++i)
          addcyc();
          
        if(cyc >= 341) cyc -= 341;

        if(bus::io().handle_input()) break;
        
      }
    
    }
  
  private:
  
    // addressing modes
    inline uint16_t ACC(){ return 0; } // template only
    inline uint16_t X__(){ return 0; }
    inline uint16_t Y__(){ return 0; }
    inline uint16_t IMM(){ return next(); }
    inline uint16_t ZPG(){ return next(); }
    inline uint16_t ZPX(){ return (next() + X)&0xff; }
    inline uint16_t ZPY(){ return (next() + Y)&0xff; }
    inline uint16_t ABS(){ return next2(); }
    inline uint16_t ABX(){ return next2() + X; }
    inline uint16_t ABY(){ return next2() + Y; }
    inline uint16_t IDX(){
      uint16_t addr = next() + X;
      return read(addr&0xff)|((uint16_t)read((addr+1)&0xff) << 8);
    }
    inline uint16_t IDY(){
      uint16_t addr = next();
      return ((uint16_t)read(addr)|(read((addr+1)&0xff)<<8))+Y;
    }
    inline uint16_t IND(){
      // When on page boundary (i.e. $xxFF) IND gets LSB from $xxFF like normal 
      // but takes MSB from $xx00
      uint16_t addr = next2();
      return addr&0xff == 0xff?
        read(addr)|(read(addr&0xff00) << 8) :
        read(addr)|(read(addr+1) << 8);
    }
    
    // some modes add cycles if page crossed
    inline uint16_t ABX_pgx(){
      return sum_check_pgx(next2(), X);
    }
  
    inline uint16_t ABY_pgx(){
      return sum_check_pgx(next2(), Y);
    }
  
    inline uint16_t IDY_pgx(){
      uint16_t addr { next() };
      return sum_check_pgx((uint16_t)read(addr)|(read((addr + 1)&0xff) << 8), Y);
    }
    
    // register read/write
    inline void Accumulator(uint8_t i){ setZN(A = i); }
    inline void IndexRegX(uint8_t i){ setZN(X = i); }
    inline void IndexRegY(uint8_t i){ setZN(Y = i); }
    inline void ProcStatus(uint8_t i){ P = (i|0x20)&~0x10; }
    inline void StackPointer(uint8_t i){ SP = i; }
    inline uint8_t Accumulator(){ return A; }
    inline uint8_t IndexRegX(){ return X; }
    inline uint8_t IndexRegY(){ return Y; }
    inline uint8_t ProcStatus(){ return P|0x10; }
    inline uint8_t StackPointer(){ return SP; }
    inline uint8_t AX(){ return A&X; } // unofficial
    
    #include "ops.cc"
    
    inline void JSR(){
      push2(PC + 1);
      PC = ABS();
    }
    
    inline void RTS(){
      PC = pull2() + 1;
    }
    
    inline void BRK(){
      push2(PC + 1);
      stack_push<&CPU::ProcStatus>();
      PC = read(0xfffe) | (read(0xffff)<<8);
    }
    
    inline void RTI(){
      stack_pull<&CPU::ProcStatus>();
      PC = pull2();
    }
    
    inline void NOP(){}
    
    inline void BAD_OP(){
      throw runtime_error("UNDEFINED OPERATION!!!");
    }
    
    static const op ops[256];
    static const char* const opasm[256];

    template<mode M> uint8_t read(){
      return read((this->*M)());
    }
    
    
};

template<> uint8_t& CPU::getref<&CPU::ACC>(){ return A; }
template<> uint8_t& CPU::getref<&CPU::X__>(){ return X; }
template<> uint8_t& CPU::getref<&CPU::Y__>(){ return Y; }

template<> uint8_t CPU::read<&CPU::IMM>(){
  return (uint8_t)IMM();
}

#include "op_table.cc"
#include "asm.cc"

class APU {



};

namespace bus {
  
  PPU _ppu;
  CPU _cpu;
  APU _apu;
  ROM _rom;
  IO _io;
  
  PPU& ppu(){    
    return _ppu;
  }
  
  CPU& cpu(){
    return _cpu;
  }
  
  APU& apu(){
    return _apu;
  }
  
  ROM& rom(){
    return _rom;
  }
  
  IO& io(){
    return _io;
  }
  
  void play(std::string const& path){
    _rom = ROM(path);
  }
  
}

int main(int argc, char* argv[]){
  
  vector<string> args (argv, argv + argc);

  try {
  
    bus::play(args[1]);
    bus::cpu().run();
  
  } catch(exception const& e){
    
    cout << e.what() << '\n';
    
  }
  
}
