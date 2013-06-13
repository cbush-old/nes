#include "ppu.h"

using std::cout;
using std::setw;
using std::hex;
using std::vector;
using std::thread;
using std::string;
using std::ifstream;
using std::exception;
using std::runtime_error;

void bisqwit_putpixel(unsigned px,unsigned py, unsigned pixel, int offset)
{
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
            static auto gammafix = [=](float f) { return f <= 0.f ? 0.f : std::pow(f, 2.2f / 1.8f); };
            static auto clamp    = [](int v) { return v>255 ? 255 : v; };
            // Store color at subpixel precision
            if(u==2) palette[o][p1][p0] += 0x10000*clamp(255 * gammafix(y/1980.f + i* 0.947f/9e6f + q* 0.624f/9e6f));
            if(u==1) palette[o][p1][p0] += 0x00100*clamp(255 * gammafix(y/1980.f + i*-0.275f/9e6f + q*-0.636f/9e6f));
            if(u==0) palette[o][p1][p0] += 0x00001*clamp(255 * gammafix(y/1980.f + i*-1.109f/9e6f + q* 1.709f/9e6f));
        }
    // Store the RGB color into the frame buffer.
    //((u32*) s->pixels) [py * 256 + px] = palette[offset][prev%64][pixel];
    
    auto x = palette[offset][prev%64][pixel];
    
    glColor3ub(
      ((x >> 16) & 0xff),
      ((x >> 8) & 0xff),
      ((x) & 0xff)
    );
  
    glVertex2i(px,py);

    prev = pixel;
    
}


template<int X, char X_MOD_8, bool TDM, bool X_ODD_64_TO_256, bool X_LT_256>
const void render2(PPU& ppu){
  #define loopy_w ppu.loopy_w
  #define pat_addr ppu.pat_addr
  #define bg_shift_pat ppu.bg_shift_pat
  #define bg_shift_attr ppu.bg_shift_attr
  #define tileattr ppu.tileattr
  #define OAM ppu.OAM
  #define ioaddr ppu.ioaddr
  #define vram ppu.vram
  #define sprinpos ppu.sprinpos
  #define sprrenpos ppu.sprrenpos
  #define sproutpos ppu.sproutpos
  #define sprtmp ppu.sprtmp
  #define scroll ppu.scroll
  #define scanline ppu.scanline
  #define OAM3 ppu.OAM3
  #define OAM2 ppu.OAM2
  #define reg ppu.reg
  #define tilepat ppu.tilepat
  #define mmap ppu.mmap
  #define scanline_end ppu.scanline_end
  #define render_pixel ppu.render_pixel
  
  if(X_MOD_8 == 2){
    ioaddr = 0x23c0 + 0x400 * vram.base_nta + 8 * (vram.ycoarse / 4) + (vram.xcoarse / 4); 
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
        (mmap(ioaddr) >> ((vram.xcoarse & 2) + 2 * (vram.ycoarse&2))) & 3;
        
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
        unsigned y = (scanline) - o.y;
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
    tilepat = mmap(pat_addr|0);
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
          int y1 = sprtmp, y2 = sprtmp + 8 + !!reg.sprite_size * 8;
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
    //if(scanline >= 0) 
      render_pixel();
  }
  #undef ioaddr
  #undef vram
  #undef sprinpos
  #undef sprrenpos
  #undef sproutpos
  #undef sprtmp
  #undef scroll
  #undef scanline
  #undef OAM3
  #undef OAM2
  #undef reg
  #undef tilepat 
  #undef mmap 
  #undef scanline_end
  #undef render_pixel
  #undef loopy_w
  #undef pat_addr
  #undef bg_shift_pat
  #undef bg_shift_attr
  #undef tileattr
  #undef OAM
}

//<int X, int X_MOD_8, int TDM, bool X_ODD_64_TO_256, bool X_LT_256>
#define X(a) render2<(\
  ((a)==0)||((a)==251)||((a)==256)||((a)==304)||((a)==337)?(a):1),\
  ((a)%8),\
  bool(0x10ffff & (1u << ((a)/16))),\
  bool(((a) & 1) && ((a) >= 64) && ((a) < 256)),\
  bool((a) < 256)>
#define Y(a) X(a),X(a+1),X(a+2),X(a+3),X(a+4),X(a+5),X(a+6),X(a+7),X(a+8),X(a+9)
const void(*render2funcs[342])(PPU&){
  Y(  0),Y( 10),Y( 20),Y( 30),Y( 40),Y( 50),Y( 60),Y( 70),Y( 80),Y( 90),
  Y(100),Y(110),Y(120),Y(130),Y(140),Y(150),Y(160),Y(170),Y(180),Y(190),
  Y(200),Y(210),Y(220),Y(230),Y(240),Y(250),Y(260),Y(270),Y(280),Y(290),
  Y(300),Y(310),Y(320),Y(330),X(340),X(341)
};
#undef X
#undef Y



void lookup_putpixel(unsigned px,unsigned py, unsigned pixel){

  glColor3b(
    ((pixel >> 4) & 0xf) * 0xf,
    ((pixel >> 2) & 0xf) * 0xf,
    ((pixel) & 0xf) * 0xf
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
#undef N_FRAMES


uint8_t& PPU::mmap(uint16_t addr){
  addr &= 0x3fff;
  if(addr < 0x2000) return bus::rom()[addr];
  if(addr < 0x3f00){
    return memory[addr&0x7ff];
    addr &= 0xfff;
    // horizontal?
    if(addr < 0x400) return memory[addr];
    if(addr < 0x800) return memory[addr];
    if(addr < 0xc00) return memory[addr - 0x800];
    return memory[addr - 0x800];
    return memory[addr&0x7ff];
  }
  //return palette[addr&3 == 0 ? addr & 0xf : addr & 0x1f];
  return palette[addr & (0xf + ((addr&3 != 0)*0x10))];
}

void PPU::render_pixel(){
  
  // this is mostly lifted from bisqwit!
  
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
  } else if((vram.raw & 0x3f00) == 0x3f00 && !reg.rendering_enabled){
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
      
      //if(cycle < 255 && pixel && s.sprindex == 0)
      if(pixel && s.sprindex == 0)
        reg.spr0_hit = true;
      
      if(!pixel || !(s.attr & 0x20)){
        attr = (s.attr & 3) + 4;
        pixel = spritepixel;
      }
      
      break;
    
    }
  }

  pixel = palette[(attr * 4 + pixel) & 0x1f] & (0x30 + (!reg.grayscale) * 0x0f);

  bisqwit_putpixel(cycle, scanline, pixel | (reg.intensify_rgb << 6), 0);
  //lookup_putpixel(cycle, scanline, pixel | (reg.intensify_rgb << 6));

}

void PPU::tick3(){
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
        render2funcs[cycle](*this);
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

PPU::PPU(): memory(0x800), palette(0x20), OAM(0x100),

  #define BAD_READ [&]{ return 0; }
  regr {
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
  },
  #undef BAD_READ


  #define BAD_WRITE [&](uint8_t){ return; }    
  regw {
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
  }
  #undef BAD_WRITE 
  
  
    
  {
  reg.PPUSTATUS = 0x80;
  glBegin(GL_POINTS);
}

PPU::~PPU(){
  glEnd();
  #ifdef DUMP_NT
  dump_nametables();
  #endif
  print_status();
  print_framerate();
}

string PPU::color(int x){
  std::stringstream ss;
  ss << "\033[";
  ss << ((x%7)+31) << ';' << ((x%8)+40) << 'm';
  return ss.str();
}

void PPU::dump_nametables(){
  
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

void PPU::print_status(){
  cout 
    << "SL:" << (int)scanline << " "
    << "CYC:" << (int)cycle << "  "
    << "VBS:" << (int)vblank_state << "  "
    << hex
    << "REG:" << (unsigned)reg.raw << "  "
    << "LPV:" << (unsigned)vram.raw << "  "
    << "LPT:" << (unsigned)scroll.raw << "\n";

}


void PPU::load_state(State const& state) {
  memory = state.ppu_memory;
  reg.raw = state.ppu_reg;
  scroll.data = state.scroll;
  vram.data = state.vram;
  read_buffer = state.read_buffer;
  vblank_state = state.vblank_state;
  loopy_w = state.loopy_w;
  NMI_pulled = state.NMI_pulled;
  
}

void PPU::save_state(State& state) const {
  state.ppu_memory = memory;
  state.ppu_reg = reg.raw;
  state.scroll = scroll.data;
  state.vram = vram.data;
  state.read_buffer = read_buffer;
  state.vblank_state = vblank_state;
  state.loopy_w = loopy_w;
  state.NMI_pulled = NMI_pulled;

}

