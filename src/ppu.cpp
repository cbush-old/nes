#include "ppu.h"

#include <SDL2/SDL.h>

using std::cout;
using std::setw;
using std::hex;
using std::vector;
using std::thread;
using std::string;
using std::ifstream;
using std::exception;
using std::runtime_error;


template<int X, char X_MOD_8, bool TDM, bool X_ODD_64_TO_256, bool X_LT_256>
void render(PPU& ppu) {
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

  #ifdef DEBUG_PPU_RENDER_TEMPLATES
  std::cout 
    << "render<" 
    << (int)X
    << ", " << (int)X_MOD_8
    << ", " << (int)TDM
    << ", " << (int)X_ODD_64_TO_256
    << ", " << (int)X_LT_256
    << ">\n";
  #endif
    
  if(X_MOD_8 == 2 && TDM){
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
      
      if(++vram.xcoarse == 0){
        vram.base_nta_x = 1 - vram.base_nta_x;
      }
        
      if(X==251){
        if(++vram.yfine == 0 && ++vram.ycoarse == 30){
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
        OAM2[sproutpos].y = sprtmp;
        OAM2[sproutpos].sprindex = reg.OAM_index;
        {
          int y1 = sprtmp, y2 = sprtmp + 8 + !!reg.sprite_size * 8;
          if(!(y1 <= scanline && scanline < y2)){
            reg.OAMADDR = sprinpos != 2 ? reg.OAMADDR + 3 : 8;
          }
        }
        break;
      case 1:
        OAM2[sproutpos].index = sprtmp;
        break;
      case 2:
        OAM2[sproutpos].attr = sprtmp;
        break;
      case 3:
        if(sproutpos < 8){
          OAM2[sproutpos++].x = sprtmp;
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
  
  if(X_LT_256 && scanline >= 0) {
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


//<int X, int X_MOD_8, int Tile_Decode_Mode, bool X_ODD_64_TO_256, bool X_LT_256>
#define X(a) render<\
  (((a)==0)||((a)==251)||((a)==256)||((a)==304)||((a)==337)?(a):1),\
  ((a) & 7),\
  bool(0x10ffff & (1u << ((a)/16))),\
  bool(((a) & 1) && ((a) >= 64) && ((a) < 256)),\
  bool((a) < 256)>
#define Y(a) X(a),X(a+1),X(a+2),X(a+3),X(a+4),X(a+5),X(a+6),X(a+7),X(a+8),X(a+9)
const std::array<std::function<void(PPU&)>, 342> renderfuncs {
  Y(  0),Y( 10),Y( 20),Y( 30),Y( 40),Y( 50),Y( 60),Y( 70),Y( 80),Y( 90),
  Y(100),Y(110),Y(120),Y(130),Y(140),Y(150),Y(160),Y(170),Y(180),Y(190),
  Y(200),Y(210),Y(220),Y(230),Y(240),Y(250),Y(260),Y(270),Y(280),Y(290),
  Y(300),Y(310),Y(320),Y(330),X(340),X(341)
};
#undef X
#undef Y


#define N_FRAMERATES 256
int framerate[N_FRAMERATES];


void print_framerate(){
  double sum = 0;
  for(int i = 0; i < N_FRAMERATES; ++i){
    sum += framerate[i];
  }
  sum /= N_FRAMERATES;
  cout << "Average framerate: " << (1000.0/sum) << "/s\n";

}

int clock_frame(){
  static int last_clock = SDL_GetTicks();
  static int i = 0;
  int tick = SDL_GetTicks();
  int d = tick - last_clock;
  framerate[i++%N_FRAMERATES] = d;
  last_clock = tick;

  #ifdef PPU_DEBUG_PRINT_FRAMERATE
  if(i%N_FRAMERATES==0)
    print_framerate();
  #endif

  return d;
}
#undef N_FRAMES


uint8_t& PPU::mmap(uint16_t addr){

  addr &= 0x3fff;

  if(addr < 0x2000) {

    return bus::rom_vbank(addr);

  } else if(addr < 0x3f00) {

    return bus::rom_nt((addr >> 10) & 3, addr & 0x3ff);

  }

  return palette[addr & (0xf | (((addr&3)!=0)<<4))];

}


static const uint32_t RGB[64] {
  0x7C7C7CFF, 0x0000FCFF, 0x0000BCFF, 0x4428BCFF, 0x940084FF, 0xA80020FF, 0xA81000FF, 0x881400FF,
  0x503000FF, 0x007800FF, 0x006800FF, 0x005800FF, 0x004058FF, 0x000000FF, 0x000000FF, 0x000000FF,
  0xBCBCBCFF, 0x0078F8FF, 0x0058F8FF, 0x6844FCFF, 0xD800CCFF, 0xE40058FF, 0xF83800FF, 0xE45C10FF,
  0xAC7C00FF, 0x00B800FF, 0x00A800FF, 0x00A844FF, 0x008888FF, 0x000000FF, 0x000000FF, 0x000000FF,
  0xF8F8F8FF, 0x3CBCFCFF, 0x6888FCFF, 0x9878F8FF, 0xF878F8FF, 0xF85898FF, 0xF87858FF, 0xFCA044FF,
  0xF8B800FF, 0xB8F818FF, 0x58D854FF, 0x58F898FF, 0x00E8D8FF, 0x787878FF, 0x000000FF, 0x000000FF,
  0xFCFCFCFF, 0xA4E4FCFF, 0xB8B8F8FF, 0xD8B8F8FF, 0xF8B8F8FF, 0xF8A4C0FF, 0xF0D0B0FF, 0xFCE0A8FF,
  0xF8D878FF, 0xD8F878FF, 0xB8F8B8FF, 0xB8F8D8FF, 0x00FCFCFF, 0xF8D8F8FF, 0x000000FF, 0x000000FF,
};

void PPU::render_pixel() {
  
  // this is so bisqwit!
  
  bool edge = uint8_t(cycle + 8) < 16; // 0..7, 248..255
  bool 
    showbg = ((!edge) || reg.show_bg8) && reg.show_bg,
    showsp = ((!edge) || reg.show_sp8) && reg.show_sp;


  unsigned 
    fx = scroll.xfine,
    xpos = 15 - (( (cycle&7) + fx + 8 * bool(cycle&7) ) & 15);
    
  unsigned pixel { 0 }, attr { 0 };
  
  if(showbg){
    pixel = (bg_shift_pat >> (xpos * 2)) & 3;
    attr = (bg_shift_attr >> (xpos * 2)) & (pixel ? 3 : 0);
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

  framebuffer[scanline * 256 + cycle] = RGB[pixel&0x3f];

}




void PPU::tick() {

  switch (vblank_state) {
    case 0: 
      if(!NMI_pulled && reg.vblanking && reg.NMI_enabled){
        bus::pull_NMI();
        NMI_pulled = true;
      }
      break;
    case 2: 
      reg.vblanking = true; 
      NMI_pulled = false; 
      break;
    case -5: 
      reg.PPUSTATUS = 0; 
      break;
  }

  if (vblank_state != 0) {
    vblank_state += (vblank_state < 0) * 2 - 1;
  }

  if (reg.rendering_enabled) {
    if (scanline < 240) {
      (*tick_renderer)(*this);
      ++tick_renderer;
    }
  }

  if(++cycle > scanline_end){

    cycle = 0;
    tick_renderer = renderfuncs.begin();
    scanline_end = 341;

    switch(++scanline){

      case 261:
        scanline = -1;
        loopy_w ^= 1;
        vblank_state = -5;
        break;

      case 241:
        // events
        if(bus::io_handle_input()) 
          throw 1;

        bus::io_swap_with(framebuffer);

        int d = 17 - clock_frame();

        SDL_Delay(d * !(d<0));
        
        vblank_state = 2;
        break;

    }
  }

}

PPU::PPU(): framebuffer(256 * 240), memory(0x800), palette(0x20), OAM(0x100), 
  tick_renderer(renderfuncs.begin()),

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
}

PPU::~PPU(){
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
  palette = state.palette;
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
  state.palette = palette;
  state.ppu_reg = reg.raw;
  state.scroll = scroll.data;
  state.vram = vram.data;
  state.read_buffer = read_buffer;
  state.vblank_state = vblank_state;
  state.loopy_w = loopy_w;
  state.NMI_pulled = NMI_pulled;
  
}

uint8_t PPU::reg_read(uint8_t i) const {
  return regr[i]();
}

void PPU::reg_write(uint8_t value, uint8_t i) {
  regw[value](i);
}

uint8_t PPU::debug_get_scanline() const {
  return scanline;
};

