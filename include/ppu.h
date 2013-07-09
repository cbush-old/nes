#ifndef PPU_H
#define PPU_H

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

#include "bit.h"
#include "bus.h"
#include "rom.h"
#include "io.h"

void lookup_putpixel(unsigned px,unsigned py, unsigned pixel);

class PPU {
  
  friend class CPU;
  
  private:
    
    std::vector<uint32_t> framebuffer;
    
    union {
      
      uint32_t raw;
      
      bit< 0,8> PPUCTRL;
      bit< 0,2> base_nta;
      bit< 2,1> vramincr;
      bit< 3,1> sp_addr;
      bit< 4,1> bg_addr;
      bit< 5,1> sprite_size;
      bit< 6,1> slave_flag;
      bit< 7,1> NMI_enabled;

      bit< 8,8> PPUMASK;
      bit< 8,1> grayscale;
      bit< 9,1> show_bg8;
      bit<10,1> show_sp8;
      bit<11,1> show_bg;
      bit<12,1> show_sp;
      bit<11,2> rendering_enabled;
      bit<13,3> intensify_rgb;
      
      bit<16,8> PPUSTATUS;
      bit<21,1> spr_overflow;
      bit<22,1> spr0_hit;
      bit<23,1> vblanking;
      
      bit<24,8> OAMADDR;
      bit<24,2> OAM_data;
      bit<26,6> OAM_index;
      
    } reg;
    
    union {
      uint32_t data;
      bit< 3,16> raw;
      bit< 0,8> xscroll;
      bit< 0,3> xfine;
      bit< 3,5> xcoarse;
      bit< 8,5> ycoarse;
      bit<13,2> base_nta;
      bit<13,1> base_nta_x;
      bit<14,1> base_nta_y;
      bit<15,3> yfine;
      bit< 3,8> vaddr_lo;
      bit<11,8> vaddr_hi;
    } scroll, vram;

    bool loopy_w { false }, NMI_pulled { false };
    
    int
      cycle { 0 }, 
      scanline { 241 },
      scanline_end { 341 },
      read_buffer { 0 },
      vblank_state { 0 };
    
    std::vector<uint8_t> memory, palette, OAM;

    unsigned pat_addr, sprinpos, sproutpos, sprrenpos, sprtmp;
    uint16_t tileattr, tilepat, ioaddr;
    uint32_t bg_shift_pat, bg_shift_attr;
    
    struct { 
      uint8_t sprindex, y, index, attr, x; 
      uint16_t pattern; 
    } OAM2[8], OAM3[8];
    
    template<int, char, bool, bool, bool>
    friend const void render2(PPU&);

    uint8_t& mmap(uint16_t addr);
    void render_pixel();
    void tick3();

    const std::vector<std::function<uint8_t()>> regr;
    const std::vector<std::function<void(uint8_t)>> regw;

  public:
    PPU();
    ~PPU();
    
    std::string color(int x);
    void dump_nametables();
    void print_status();
    void load_state(State const&);
    void save_state(State&) const;
    
    
};


#endif
