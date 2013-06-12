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

using std::cout;
using std::setw;
using std::hex;
using std::vector;
using std::thread;
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
  void pull_NMI();
}

// CPU cycle chart
static const uint8_t cycles[256] {
//////// 0 1 2 3 4 5 6 7 8 9 A B C D E F
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

void bisqwit_putpixel(unsigned px,unsigned py, unsigned pixel, int offset){
  // The input value is a NES color index (with de-emphasis bits).
  // We need RGB values. To produce a RGB value, we emulate the NTSC circuitry.
  // For most part, this process is described at:
  //    http://wiki.nesdev.com/w/index.php/NTSC_video
  // Incidentally, this code is shorter than a table of 64*8 RGB values.
  static unsigned palette[3][64][512] = {}, prev=~0u;
  // Caching the generated colors
  if(prev == ~0u)
      for(int o=0; o<3; ++o)
      for(int u=0; u<3; ++u)
      for(int p0=0; p0<512; ++p0)
      for(int p1=0; p1<64; ++p1)
      {
          // Calculate the luma and chroma by emulating the relevant circuits:
          auto s = "\372\273\32\305\35\311I\330D\357}\13D!}N";
          int y=0, i=0, q=0;
          for(int p=0; p<12; ++p) // 12 samples of NTSC signal constitute a color.
          {
              // Sample either the previous or the current pixel.
              int r = (p+o*4)%12, pixel = r < 8-u*2 ? p0 : p1; // Use pixel=p0 to disable artifacts.
              // Decode the color index.
              int c = pixel%16, l = c<0xE ? pixel/4 & 12 : 4, e=p0/64;
              // NES NTSC modulator (square wave between up to four voltage levels):
              int b = 40 + s[(c > 12*((c+8+p)%12 < 6)) + 2*!(0451326 >> p/2*3 & e) + l];
              // Ideal TV NTSC demodulator:
              y += b;
              i += b * int(std::cos(M_PI * p / 6) * 5909);
              q += b * int(std::sin(M_PI * p / 6) * 5909);
          }
          // Convert the YIQ color into RGB
          auto gammafix = [=](float f) { return f <= 0.f ? 0.f : std::pow(f, 2.2f / 1.8f); };
          auto clamp    = [](int v) { return v>255 ? 255 : v; };
          // Store color at subpixel precision
          if(u==2) palette[o][p1][p0] += 0x100000*clamp(255 * gammafix(y/1980.f + i* 0.947f/9e6f + q* 0.624f/9e6f));
          if(u==1) palette[o][p1][p0] += 0x001000*clamp(255 * gammafix(y/1980.f + i*-0.275f/9e6f + q*-0.636f/9e6f));
          if(u==0) palette[o][p1][p0] += 0x000010*clamp(255 * gammafix(y/1980.f + i*-1.109f/9e6f + q* 1.709f/9e6f));
      }
  // Store the RGB color into the frame buffer.
  //((u32*) s->pixels) [py * 256 + px] = palette[offset][prev%64][pixel];
  glColor3b(
    (palette[offset][prev%64][pixel] >> 8) & 0xff,
    (palette[offset][prev%64][pixel] >> 16) & 0xff,
    (palette[offset][prev%64][pixel] >> 24) & 0xff
  );
  
  glVertex2i(px,py);
  
  prev = pixel;
}

void lookup_putpixel(unsigned px,unsigned py, unsigned pixel){
  static uint16_t palette[64] {
    0x333, 0x002, 0x102, 0x203, 0x303, 0x300, 0x320, 0x200, 0x230, 0x030, 0x020, 0x010, 0x114, 0x000, 0x000, 0x000,
    0x666, 0x006, 0x306, 0x406, 0x606, 0x600, 0x640, 0x400, 0x460, 0x060, 0x040, 0x020, 0x228, 0x000, 0x000, 0x000,
    0xccc, 0x00c, 0x60c, 0x80c, 0xc0c, 0xc00, 0xc80, 0x800, 0x8c0, 0x0c0, 0x080, 0x040, 0x44e, 0x333, 0x000, 0x000,
    0xeee, 0xaae, 0xeae, 0xeac, 0xfef, 0xeee, 0xef0, 0xffe, 0xddd, 0xafa, 0xaff, 0xfff, 0xffe, 0xfff, 0x000, 0x000
  };
  
  glColor3b(
    ((palette[64-pixel] >> 8) & 0xf) * 0xf,
    ((palette[64-pixel] >> 4) & 0xf) * 0xf,
    ((palette[64-pixel]) & 0xf) * 0xf
  );
  
  glVertex2i(px,py);
  
}

#define N_FRAMERATES 256
double framerate[N_FRAMERATES];
void print_framerate(){
  
  double sum = 0;
  for(int i = 0; i < N_FRAMERATES; ++i){
    sum += framerate[i];
  }
  sum /= N_FRAMERATES;
  cout << "Average framerate: " << (1.0/(sum/CLOCKS_PER_SEC)) << "/s\n";

}

void clock_frame(){
  static clock_t last_clock { 0 };
  static int i { 0 };
  clock_t tick = clock();
  framerate[i++%N_FRAMERATES] = difftime(tick, last_clock);
  last_clock = tick;
  if(i%N_FRAMERATES==0)
    print_framerate();
}


// sexy bisqwit
template<size_t position, size_t length=1, typename T=uint32_t>
struct bit {
  T value;
  constexpr unsigned mask(){ return (1u << length) - 1u; };
  constexpr unsigned not_mask_at_pos(){ return ~(mask() << position); };
  template<typename T2>
  bit& operator= (T2 that){
    value &= not_mask_at_pos();
    value |= (that & mask()) << position;
  }
  operator unsigned() const {
    return (value >> position) & mask();
  }
  bit& operator++(){
    return *this = *this + 1;
  }
  unsigned operator++(int){
    unsigned r = *this;
    ++*this;
    return r;
  }
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
        //512,480,SDL_WINDOW_OPENGL
        256,240,SDL_WINDOW_OPENGL
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
              #define BUTTON_ON 0xff
              case SDLK_RIGHT: button_state[0] = BUTTON_ON; break;
              case SDLK_LEFT:  button_state[1] = BUTTON_ON; break;
              case SDLK_DOWN:  button_state[2] = BUTTON_ON; break;
              case SDLK_UP:    button_state[3] = BUTTON_ON; break;
              case SDLK_r:  button_state[4] = BUTTON_ON; break;
              case SDLK_s:  button_state[5] = BUTTON_ON; break;
              case SDLK_d:  button_state[6] = BUTTON_ON; break;
              case SDLK_f:  button_state[7] = BUTTON_ON; break;
              #undef BUTTON_ON
            }
            return 0;
          case SDL_KEYUP:
            switch(e.key.keysym.sym){
              case SDLK_RIGHT: button_state[0] = 0x0; break;
              case SDLK_LEFT:  button_state[1] = 0x0; break;
              case SDLK_DOWN:  button_state[2] = 0x0; break;
              case SDLK_UP:    button_state[3] = 0x0; break;
              case SDLK_r:    button_state[4] = 0x0; break;
              case SDLK_s:   button_state[5] = 0x0; break;
              case SDLK_d:  button_state[6] = 0x0; break;
              case SDLK_f: button_state[7] = 0x0; break;
            }
            return 0;
          case SDL_QUIT:
            return 1;
        }
      }
    }
    
    int button_index { 0 };
    uint8_t button_state[8];
    
    uint8_t input_state(uint8_t i){
      if(i == 2) return 0;
      if(button_index == -1)
        return 1;
      return button_state[button_index--];
    }
    
    void strobe(){
      button_index = 7;
    }
    
    void swap(){
      SDL_GL_SwapWindow(window);
      glClear(GL_COLOR_BUFFER_BIT);
    }
    
    void clear(){
      glClear(GL_COLOR_BUFFER_BIT);
      SDL_GL_SwapWindow(window);
    }
    
    void put_pixel(int x, int y, char r, char g, char b){
      glColor3b(r,g,b);
      glBegin(GL_POINTS);
      glVertex2i(x,y);
      glEnd();
      /*
      glBegin(GL_QUADS);
      glVertex2i(x,y);
      glVertex2i(x+1,y);
      glVertex2i(x+1,y+1);
      glVertex2i(x,y+1);
      glEnd();
      */
    }
    
  public:
    IO(){
      glMatrixMode(GL_PROJECTION|GL_MODELVIEW);
      glLoadIdentity();
      glOrtho(0,256,240,0,0,1);
      glClearColor(0.2,0.2,0.2,1);
      glClear(GL_COLOR_BUFFER_BIT);
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
      bit<13,1> intensify_red;
      bit<14,1> intensify_green;
      bit<15,1> intensify_blue;
      
      bit<16,8> PPUSTATUS;
      bit<21,1> spr_overflow;
      bit<22,1> spr0_hit;
      bit<23,1> vblanking;
      
      bit<24,8> OAMADDR;
      bit<24,2> OAM_data;
      bit<26,6> OAM_index;
      
    } reg;
    
    uint8_t palette[0x20], OAM[0x100];

    union {
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
    
    std::vector<uint8_t> memory;
      
    uint8_t& mmap(uint16_t addr){
      addr &= 0x3fff;
      if(addr < 0x2000) return bus::rom()[addr];
      if(addr < 0x3f00) return memory[addr&0x7ff];
      //return palette[addr&3 == 0 ? addr & 0xf : addr & 0x1f];
      return palette[addr & (0xf + ((!(addr&3 == 0))*0x10))];
    }

    void render_pixel(){

      bool edge = uint8_t(cycle + 8) < 16;
      bool 
        showbg = (!edge || reg.show_bg8) && reg.show_bg,
        showsp = (!edge || reg.show_sp8) && reg.show_sp;
      
      unsigned fx = scroll.xfine,
        xpos = 15 - (( (cycle&7) + fx + 8 * bool(cycle&7) ) & 15);
        
      unsigned pixel { 0 }, attr { 0 };
      if(showbg){
        pixel = (bg_shift_pat >> (xpos * 2)) & 3;
        attr = (bg_shift_attr >> (xpos * 2)) & (!!pixel * 3);
      } else if(
        (vram.raw & 0x3f00) == 0x3f00 && !reg.rendering_enabled
      ){
        pixel = vram.raw;
      }
      
      if(showsp){
        for(unsigned sno = 0; sno < sprrenpos; ++sno){
          auto& s = OAM3[sno];
          
          unsigned xdiff = cycle - s.x;
          
          if(xdiff >= 8)
            continue;
            
          if(!(s.attr & 0x40)) 
            xdiff = 7 - xdiff;
          
          uint8_t spritepixel = (s.pattern >> (xdiff * 2)) & 3;
          
          if(!spritepixel) 
            continue;
          
          if(cycle < 255 && pixel && s.sprindex == 0)
            reg.spr0_hit = true;
          
          if(!pixel || !(s.attr & 0x20)){
            attr = (s.attr & 3) + 4;
            pixel = spritepixel;
          }
          
          break;
        
        }
      }
    
      pixel = palette[(attr * 4 + pixel) & 0x1f] & (0x30 + !reg.grayscale * 0xf);
      
      //bisqwit_putpixel(cycle, scanline, pixel, 0);
      lookup_putpixel(cycle, scanline, pixel);

    }
    
    #ifdef RENDER1
    #include "render1.cc"
    #else
    
    template<int X, int X_MOD_8, int TDM, bool X_ODD_64_TO_256, bool X_LT_256>
    const void render2(){
      if(X_MOD_8 == 2){
        ioaddr = 0x23c0 + 0x400 * vram.base_nta + 8 * vram.ycoarse / 4 + vram.xcoarse / 4; 
      } 
      
      if(X_MOD_8 == 0 || (X_MOD_8 == 2 && !TDM)){
        ioaddr = 0x2000 + (vram.raw & 0xfff);
        
        if(X == 0){
          sprinpos = sproutpos = 0;
          if(reg.show_sp)
            reg.OAMADDR = 0;
        }
        
        if(X == 256){
          if(reg.show_bg){
            vram.xcoarse = (unsigned)scroll.xcoarse;
            vram.base_nta_x = (unsigned)scroll.base_nta_x;
            sprrenpos = 0;
          }
        }
        
        if(X == 304){
          if(scanline == -1 && reg.show_bg)
            vram.raw = (unsigned)scroll.raw;
        }
        
      } 
      
      if(X_MOD_8 == 1){
        if(X==337){
          if(scanline == -1 && loopy_w && reg.show_bg){
            scanline_end = 340;
          }
        }
        pat_addr = 0x1000 * reg.bg_addr + 16 * mmap(ioaddr) + vram.yfine;
        if(TDM){
          bg_shift_pat = (bg_shift_pat >> 16) + 0x00010000 * tilepat;
          bg_shift_attr = (bg_shift_attr >> 16) + 0x55550000 * tileattr;
        }
      }
      
      if(X_MOD_8 == 3){
        if(TDM){
          tileattr = 
            (mmap(ioaddr) >> 
            ((vram.xcoarse & 2) + 2 * (vram.ycoarse&2))) & 3;
            
          if(!++vram.xcoarse) 
            vram.base_nta_x = 1 - vram.base_nta_x;
            
          if(X==251){
            if(!++vram.yfine && ++vram.ycoarse == 30){
              vram.ycoarse = 0;
              vram.base_nta_y = 1 - vram.base_nta_y;
            }
          }
        } else {
          if(sprrenpos < sproutpos){
            auto& o = OAM3[sprrenpos];
            memcpy(&o, &OAM2[sprrenpos], sizeof(o));
            unsigned y = scanline - o.y;
            if(o.attr & 0x80){
              y ^= 7 + reg.sprite_size * 8;
            }
            if(reg.sprite_size){
              pat_addr = 
                0x1000 * (o.index & 0x01)
                + 0x10 * (o.index & 0xfe);
            } else {
              pat_addr = 
                0x1000 * reg.sp_addr
                + 0x10 * (o.index & 0xff);
            }
            pat_addr += (y&7) + (y&8) * 2;
          }
        }
        
      }
      
      if(X_MOD_8 == 5){
        tilepat = mmap(pat_addr);
      } 
      
      if(X_MOD_8 == 7){
        unsigned p = tilepat | (mmap(pat_addr|8) << 8);
        p = (p&0xf00f) | ((p&0x0f00)>>4) | ((p&0x00f0)<<4);
        p = (p&0xc3c3) | ((p&0x3030)>>2) | ((p&0x0c0c)<<2);
        p = (p&0x9999) | ((p&0x4444)>>1) | ((p&0x2222)<<1);
        tilepat = p;
        if(!TDM){
          if(sprrenpos < sproutpos)
            OAM3[sprrenpos++].pattern = tilepat;

      }
      
      if(X_ODD_64_TO_256){
        switch(reg.OAMADDR++&3){
          case 0:
            if(sprinpos >= 64){
              reg.OAMADDR = 0;
              break;
            }
            ++sprinpos;
            if(sproutpos < 8){
              OAM2[sproutpos].y = sprtmp;
              OAM2[sproutpos].sprindex = reg.OAM_index;
            }
            {
              int y1 = sprtmp, y2 = sprtmp + 8 + reg.sprite_size * 8;
              if(!(y1 <= scanline && scanline < y2)){
                reg.OAMADDR = sprinpos != 2 ? reg.OAMADDR + 3 : 8;
              }
            }
            break;
          case 1:
            if(sproutpos < 8)
              OAM2[sproutpos].index = sprtmp;
            break;
          case 2:
            if(sproutpos < 8)
              OAM2[sproutpos].attr = sprtmp;
            break;
          case 3:
            if(sproutpos < 8){
              OAM2[sproutpos].x = sprtmp;
              ++sproutpos;
            } else {
              reg.spr_overflow = true;
            }
            
            if(sprinpos == 2)
              reg.OAMADDR = 8;
            
            break;
        }
      } else {
        sprtmp = OAM[reg.OAMADDR];
      }
      
      if(X_LT_256){
        if(scanline >= 0) 
          render_pixel();
      }
      
    }
    
    
    //<int X, int X_MOD_8, int TDM, bool X_ODD_64_TO_256, bool X_LT_256>
    #define X(a) &PPU::render2<(\
      ((a)==0)||((a)==251)||((a)==256)||((a)==304)||((a)==337)?(a):1),\
      ((a)%8),\
      bool(0x10ffff & (1u << ((a)/16))),\
      bool(((a) & 1) && ((a) >= 64) && ((a) < 256)),\
      bool((a) < 256)>
    #define Y(a) X(a),X(a+1),X(a+2),X(a+3),X(a+4),X(a+5),X(a+6),X(a+7),X(a+8),X(a+9)
    const void(PPU::*render2funcs[342])(){
      Y(  0),Y( 10),Y( 20),Y( 30),Y( 40),Y( 50),Y( 60),Y( 70),Y( 80),Y( 90),
      Y(100),Y(110),Y(120),Y(130),Y(140),Y(150),Y(160),Y(170),Y(180),Y(190),
      Y(200),Y(210),Y(220),Y(230),Y(240),Y(250),Y(260),Y(270),Y(280),Y(290),
      Y(300),Y(310),Y(320),Y(330),X(340),X(341)
    };
    #undef X
    #undef Y
    #endif
    
    inline void tick3(){
      for(int i = 0; i < 3; ++i){
      switch(vblank_state){
        case -5: 
          reg.PPUSTATUS = 0; 
          break;
        case 2: 
          reg.vblanking = true; 
          NMI_pulled = false; 
          break;
        case 0: 
          if(!NMI_pulled && reg.vblanking && reg.NMI_enabled){
            bus::pull_NMI();
            NMI_pulled = true;
          }
          break;
      }
      
      if(vblank_state != 0){
        vblank_state += (vblank_state < 0) * 2 - 1;
      }
      
      if(reg.rendering_enabled){
        if(scanline < 240){
          #ifdef RENDER1
          (this->*(renderfs[cycle]))();
          #else
          (this->*(render2funcs[cycle]))();
          #endif
        }
      }
      
      if(++cycle > scanline_end){
        //bus::io().swap();
        cycle = 0;
        scanline_end = 341;
        switch(++scanline){
          case 261:
            scanline = -1;
            loopy_w ^= 1;
            vblank_state = -5;
            break;
          case 241:
            // events
            if(bus::io().handle_input()) 
              throw 1;
            glEnd();
            bus::io().swap();
            glBegin(GL_POINTS);
            
            clock_frame();
            
            vblank_state = 2;
            break;
        }
      }
      }
    }

    unsigned pat_addr, sprinpos, sproutpos, sprrenpos, sprtmp;
    uint16_t tileattr, tilepat, ioaddr;
    uint32_t bg_shift_pat, bg_shift_attr;
    
    bool tile_decode_mode;
    
    struct { 
      uint8_t sprindex, y, index, attr, x; 
      uint16_t pattern; 
    } OAM2[8], OAM3[8];
    
    
    #define BAD_READ [&]{ return 0; }
    const std::vector<std::function<uint8_t()>> regr {
      /* 0 */ BAD_READ,
      /* 1 */ BAD_READ,
      /* 2 */ [&]{
        uint8_t result { reg.PPUSTATUS };
        reg.vblanking = false;
        loopy_w = false;
        return result;
      },
      /* 3 */ BAD_READ,
      /* 4 */ [&]{
        return OAM[reg.OAMADDR] & (reg.OAM_data == 2 ? 0xE3 : 0xFF);
      },
      /* 5 */ BAD_READ,
      /* 6 */ BAD_READ,
      /* 7 */ [&]{
        uint8_t result = read_buffer;
        read_buffer = mmap(vram.raw);
        vram.raw = vram.raw + (!!reg.vramincr * 31 + 1);
        return result;
      }
    };
    #undef BAD_READ

    #define BAD_WRITE [&](uint8_t){ return; }    
    const std::vector<std::function<void(uint8_t)>> regw {
      /* 0 */ [&](uint8_t x) {
        reg.PPUCTRL = x; 
        scroll.base_nta = reg.base_nta; 
      },
      /* 1 */ [&](uint8_t x) { reg.PPUMASK = x; },
      /* 2 */ BAD_WRITE,
      /* 3 */ [&](uint8_t x) { reg.OAMADDR = x; },
      /* 4 */ [&](uint8_t x) { OAM[reg.OAMADDR++] = x; },
      /* 5 */ [&](uint8_t x) { 
        if(loopy_w){
          scroll.yfine = x & 7;
          scroll.ycoarse = x >> 3;
        } else {
          scroll.xscroll = x;
        }
        loopy_w ^= 1;
      },
      /* 6 */ [&](uint8_t x) {
        if(loopy_w){
          scroll.vaddr_lo = x;
          vram.raw = (unsigned)scroll.raw;
        } else {
          scroll.vaddr_hi = x & 0x3f;
        }
        loopy_w ^= 1;
      },
      /* 7 */ [&](uint8_t x) {
        mmap(vram.raw) = x;
        vram.raw = vram.raw + (bool(reg.vramincr) * 31 + 1);
      }
    };
    #undef BAD_WRITE
  
  public:
    PPU():memory(0x800){
      reg.PPUSTATUS = 0x80;
      glBegin(GL_POINTS);
    }
    ~PPU(){
      glEnd();
      #ifdef DUMP_NT
      dump_nametables();
      #endif
      print_status();
      print_framerate();
      
    }
    
    string color(int x){
      std::stringstream ss;
      ss << "\033[";
      ss << ((x%7)+31) << ';' << ((x%8)+40) << 'm';
      return ss.str();
    }
    
    void dump_nametables(){
      cout << '\n';
      
      for(int i = 0x2000; i < 0x2400; ++i){
        //if((i&63) > 31) continue;
          
        int x = mmap(i);
        cout 
          << color(x)
          << setw(3) 
          << std::setfill(' ') 
          << hex 
          << x 
          << "\033[0m";
        
        if((i%32)==31)
          cout << "\n";
          
      }
    
    }
    
    void print_status(){
      cout 
        << "SL:" << (int)scanline << " "
        << "CYC:" << (int)cycle << "  "
        << "VBS:" << (int)vblank_state << "  "
        << hex
        << "REG:" << (unsigned)reg.raw << "  "
        << "LPV:" << (unsigned)vram.raw << "  "
        << "LPT:" << (unsigned)scroll.raw << "\n";
    
    }
    
    vector<std::function<void()>> tests {
      [&]{
        scroll.base_nta = 1;
        cout << (int)scroll.base_nta << '\n';
      },
    };
    
    void test(){
      
      
      for(auto& f : tests){
        f();
        print_status();
      }
      
    }
    
  
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
    
    uint8_t read(uint16_t addr){
      if(addr < 0x2000) return memory[addr&0x7ff];
      if(addr < 0x4000){
        auto x = bus::ppu().regr[addr&7]();
        #ifdef DEBUG_PPU
        cout << "Read PPU " << (addr&7) << " --> " << (int)x << '\n';
        #endif
        return x;
      }
      if(addr < 0x4020){
        switch(addr&0x1f){
          case 0x15:
          case 0x16: return bus::io().input_state(1);
          case 0x17: return bus::io().input_state(2);
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
      if(addr < 0x4000){
        #ifdef DEBUG_PPU
        cout << "Write PPU " << (addr&7) << " value " << (int)value << '\n';
        #endif
        return bus::ppu().regw[addr&7](value), 0;
      }
      if(addr < 0x4020){
        switch(addr&0x1f){
          case 0x14: {
            // DMA transfer
            PPU& ppu = bus::ppu();
            for(int i=0; i < 256; ++i){
              ppu.regw[4](read((value&7)*0x100 + i));
            }
          } break;
          case 0x16: 
            if(value&1) 
              bus::io().strobe();
            break;
          default: return 0;
          
        }
      }
      if(addr < 0x8000){
        // The alternate output...
        /*
        if(!value) cout << '\n';
        cout << hex << std::setw(2) << std::setfill(' ') << value << ' ';
        */
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
      return r | (memory[++SP + 0x100] << 8);    
    }
    
    void addcyc(){
      //cyc += 3;
      ++result_cycle;
//      bus::ppu().tick3();
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
      stack_push<&CPU::ProcStatus>();
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
          << " SL:" << setw(3) << (int)bus::ppu().scanline
          << hex << std::setfill('0')
          << " ST0:" << setw(2) << (int)memory[0x101 + SP]
          << " ST1:" << setw(2) << (int)memory[0x102 + SP]
          << " ST2:" << setw(2) << (int)memory[0x103 + SP]
          << '\n';
        //if(cyc >= 341) cyc -= 341;
#endif
        
        (this->*ops[last_op])();
      
        for(int i = 0; i < cycles[last_op]; ++i)
          bus::ppu().tick3();
            
        for(int i = 0; i < result_cycle; ++i)
          bus::ppu().tick3();
          
        result_cycle = 0;
          
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
  
  IO _io;
  PPU _ppu;
  CPU _cpu;
  APU _apu;
  ROM _rom;

  
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
  
  void pull_NMI(){
    _cpu.pull_NMI();
  }
  
  void play(std::string const& path){
    _rom = ROM(path);
  }
  
}

int main(int argc, char* argv[]){

  vector<string> args (argv, argv + argc);

  try {
    bus::play(args[1]);
  #ifndef PPU_TEST
    
    bus::cpu().run();
    
  #else
    
    bus::ppu().test();
    
  #endif
  
  } catch(exception const& e){
    
    cout << e.what() << '\n';
    
  } catch(...){
  
    cout << "died\n";
    
  }
  
}
