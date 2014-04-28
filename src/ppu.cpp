#include "ppu.h"
#include <thread>
#include <chrono>

#include <SDL2/SDL.h>

// So much of this comes from bisqwit's NES.

const uint16_t ATTRIBUTE_TABLE_BASE_ADDR = 0x23c0;
const uint16_t NAME_TABLE_BASE_ADDR = 0x2000;
const uint16_t NAME_TABLE_SIZE = 0x400;
const uint16_t PATTERN_TABLE_SIZE = 0x1000;

template<int X, char X_MOD_8, bool TDM, bool X_ODD_64_TO_256, bool X_LT_256>
void PPU::render() {

  // Prepare fetch pattern from the NT
  // Also, 2nd fetch in cycle is another NT byte at the end of the scanline
  if(X_MOD_8 == 0 || (X_MOD_8 == 2 && !TDM)) {

    ioaddr = NAME_TABLE_BASE_ADDR + (vram.raw & 0xfff);

  }

  // Fetch pattern byte
  if (X_MOD_8 == 1) {

    pat_addr = PATTERN_TABLE_SIZE * reg.bg_addr + 16 * read(ioaddr) + vram.yfine;

    if (TDM) {
      // Simulate the per-cycle shifting and re-fill of the registers.
      // Here the registers are just shifted and filled at once.
      bg_shift_pat = (bg_shift_pat >> 16) + (tilepat << 16);
      bg_shift_attr = (bg_shift_attr >> 16) + 0x55550000 * tileattr;
    }
  }

  // Prepare fetch attribute byte
  if (X_MOD_8 == 2 && TDM) {
    ioaddr = ATTRIBUTE_TABLE_BASE_ADDR + NAME_TABLE_SIZE * vram.base_nta + 8 * (vram.ycoarse / 4) + (vram.xcoarse / 4); 
  } 

  // Fetch attribute
  if (X_MOD_8 == 3 && TDM) {

    tileattr = (read(ioaddr) >> ((vram.xcoarse & 2) + 2 * (vram.ycoarse&2))) & 3;

    if (++vram.xcoarse == 0) {
      vram.base_nta_x = 1 - vram.base_nta_x;
    }

  }

  // Fetch lower tile byte
  if(X_MOD_8 == 5) {
    tilepat = read(pat_addr|0);
  }
  
  // Fetch upper tile byte
  if(X_MOD_8 == 7) {
    unsigned p = tilepat | (read(pat_addr | 8) << 8);

    // Shuffle the two patterns together
    p = (p & 0xf00f) | ((p & 0x0f00)>>4) | ((p & 0x00f0)<<4);
    p = (p & 0xc3c3) | ((p & 0x3030)>>2) | ((p & 0x0c0c)<<2);
    p = (p & 0x9999) | ((p & 0x4444)>>1) | ((p & 0x2222)<<1);
    tilepat = p;

  }



  // First cycle: reset
  if (X == 0) {
    sprinpos = sproutpos = 0;
    if(reg.show_sp) {
      reg.OAMADDR = 0;
    }
  }

  // End of scanline: copy temp horizontal data to main vram
  if (X == 256) {
    if (reg.show_bg) {
      vram.xcoarse = (unsigned)scroll.xcoarse;
      vram.base_nta_x = (unsigned)scroll.base_nta_x;
      sprrenpos = 0;
    }
  }

  // Increment y
  if (X == 251) {
    if (++vram.yfine == 0 && ++vram.ycoarse == 30) {
      vram.ycoarse = 0;
      vram.base_nta_y = 1 - vram.base_nta_y;
    }
  }

  // Pre-render scanline: copy temp vertical data to main vram
  if (X == 304) {
    if(scanline == -1 && reg.show_bg)
      vram.raw = (unsigned)scroll.raw;
  }

  // Alternate frames skip one cycle on pre-render scanline if bg is enabled
  if(X == 337) {
    if(scanline == -1 && loopy_w && reg.show_bg){
      scanline_end = 340;
    }
  }



  // Sprite update stuff
  //
  //
  if (X_MOD_8 == 3 && !TDM) {

    if(sprrenpos < sproutpos) {

      auto& object = OAM3[sprrenpos];
      memcpy(&object, &OAM2[sprrenpos], sizeof(object));
      
      unsigned y = scanline - object.y;

      if (object.flip_vertically) {
        y ^= (reg.sprite_size + 1) * 8 - 1;
      }

      if (reg.sprite_size) {

        pat_addr = PATTERN_TABLE_SIZE * (object.index & 1) + 0x10 * (object.index & 0xfe);

      } else {

        pat_addr = PATTERN_TABLE_SIZE * reg.sp_addr + 0x10 * (object.index & 0xff);

      }

      pat_addr += (y & 7) + (y & 8) * 2;

    }

  }

  // Ready sprite
  //
  if (X_MOD_8 == 7 && !TDM) {
    if (sprrenpos < sproutpos) {
      OAM3[sprrenpos++].pattern = tilepat;
    }
  }


  // Prepare sprite data
  //
  if (X_ODD_64_TO_256) {
    switch (reg.OAMADDR++ & 3) {
      case 0:
        // Primary OAM can hold 64 sprites per frame
        if (sprinpos >= 64) {
          reg.OAMADDR = 0;
          break;
        }

        ++sprinpos;

        // Secondary OAM holds max 8 sprites per scanline
        OAM2[sproutpos].y = sprtmp;
        OAM2[sproutpos].sprindex = reg.OAM_index;
        
        // If the sprite is not on this scanline, skip ahead in memory
        if (!(sprtmp <= scanline && scanline < sprtmp + (reg.sprite_size? 16 : 8))) {
          reg.OAMADDR = sprinpos != 2 ? reg.OAMADDR + 3 : 8;
        }

        break;

      case 1:
        OAM2[sproutpos].index = sprtmp;
        break;

      case 2:
        OAM2[sproutpos].attr = sprtmp;
        break;

      case 3:
        if (sproutpos < 8) {
          OAM2[sproutpos++].x = sprtmp;
        } else {
          reg.spr_overflow = true;
        }
        
        if (sprinpos == 2) {
          reg.OAMADDR = 8;
        }
        
        break;
    }
  } else {

    sprtmp = OAM[reg.OAMADDR];

  }

  if(X_LT_256 && scanline >= 0) {
    render_pixel();
  }

}


//<int X, int X_MOD_8, int Tile_Decode_Mode, bool X_ODD_64_TO_256, bool X_LT_256>
#define X(a) &PPU::render<\
  (((a)==0)||((a)==251)||((a)==256)||((a)==304)||((a)==337)?(a):1),\
  ((a) & 7),\
  bool(0x10ffff & (1u << ((a)/16))),\
  bool(((a) & 1) && ((a) >= 64) && ((a) < 256)),\
  bool((a) < 256)>

#define Y(a) X(a),X(a+1),X(a+2),X(a+3),X(a+4),X(a+5),X(a+6),X(a+7),X(a+8),X(a+9)
const PPU::Renderf_array PPU::renderfuncs {
  Y(  0),Y( 10),Y( 20),Y( 30),Y( 40),Y( 50),Y( 60),Y( 70),Y( 80),Y( 90),
  Y(100),Y(110),Y(120),Y(130),Y(140),Y(150),Y(160),Y(170),Y(180),Y(190),
  Y(200),Y(210),Y(220),Y(230),Y(240),Y(250),Y(260),Y(270),Y(280),Y(290),
  Y(300),Y(310),Y(320),Y(330),X(340),X(341)
};
#undef X
#undef Y



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

  bool edge = !(8 <= cycle && cycle < 248);
  bool showbg = ((!edge) || reg.show_bg8) && reg.show_bg;
  bool showsp = ((!edge) || reg.show_sp8) && reg.show_sp;

  unsigned fx = scroll.xfine;
  uint8_t xpos = 15 - (( (cycle&7) + fx + 8 * bool(cycle&7) ) & 15);
    
  unsigned pixel { 0 }, attr { 0 };
  
  if (showbg) {
    pixel = (bg_shift_pat >> (xpos * 2)) & 3;
    attr = (bg_shift_attr >> (xpos * 2)) & (pixel ? 3 : 0);
  } else if ((vram.raw & 0x3f00) == 0x3f00 && !reg.rendering_enabled) {
    pixel = vram.raw;
  }

  if (showsp) {
    for(unsigned sno = 0; sno < sprrenpos; ++sno){
      auto& s = OAM3[sno];
      
      unsigned xdiff = cycle - s.x;
      
      if(xdiff >= 8)
        continue;
        
      if(!s.flip_horizontally) {
        xdiff = 7 - xdiff;
      }

      uint8_t spritepixel = (s.pattern >> (xdiff * 2)) & 3;

      if(!spritepixel) 
        continue;

      if(pixel && s.sprindex == 0)
        reg.spr0_hit = true;

      if (!pixel || !s.priority) {
        attr = s.palette + 4;
        pixel = spritepixel;
      }
      
      break;
    
    }
  }

  pixel = palette[(attr * 4 + pixel) & 0x1f] & (0x30 + (!reg.grayscale) * 0x0f);

  framebuffer[scanline * 256 + cycle] = RGB[pixel&0x3f];

}






#define N_FRAMERATES 256
int framerate[N_FRAMERATES];


void print_framerate() {
  double sum = 0;
  for(int i = 0; i < N_FRAMERATES; ++i){
    sum += framerate[i];
  }
  sum /= N_FRAMERATES;
  std::cout << "Average framerate: " << (1000.0/sum) << "/s\n";

}

int clock_frame() {
  static int last_clock = SDL_GetTicks();
  static int i = 0;
  int tick = SDL_GetTicks();
  int d = tick - last_clock;
  framerate[i++%N_FRAMERATES] = d;
  last_clock = tick;

  #ifdef DEBUG_PPU_PRINT_FRAMERATE
  if(i%N_FRAMERATES==0)
    print_framerate();
  #endif

  return d;
}
#undef N_FRAMES


void PPU::write(uint8_t value, uint16_t addr) {

  addr &= 0x3fff;

  if (addr < 0x2000) { // Pattern table (CHR RAM/ROM)

    rom->write_chr(value, addr);

  } else if (addr < 0x3f00) { // Name table

    rom->write_nt(value, addr - 0x2000);

  } else { // Palette

    palette[addr & (0xf | (((addr & 3) != 0) << 4))] = value;

  }

}

// http://wiki.nesdev.com/w/index.php/PPU_memory_map
uint8_t PPU::read(uint16_t addr, bool no_palette /* = false */) const {

  addr &= 0x3fff;

  if (addr < 0x2000) { // Pattern table

    return rom->read_chr(addr);

  } else if (addr < 0x3f00 + no_palette * 0xff) { // Name table

    return rom->read_nt(addr - 0x2000);

  } else { // Palette http://wiki.nesdev.com/w/index.php/PPU_palettes

    // Retrieve the palette index; address above 0x3f20 are mirrors of 0x3f00 to 0x3f1f.
    // Additionally, index 10, 14, 18 and 1C are mirrors of 0, 4, 8 and C respectively.
    return palette[addr & (0xf | (((addr & 3) != 0) << 4))];

  }

}



void PPU::tick() {

  switch (vblank_state) {
    case -5: 
      reg.PPUSTATUS = 0; 
      break;
    case 0:
      if(!NMI_pulled && reg.vblanking && reg.NMI_enabled){
        bus->pull_NMI();
        NMI_pulled = true;
      }
      break;
    case 2:
      reg.vblanking = true; 
      NMI_pulled = false; 
      break;
  }

  if (vblank_state < 0) {
    ++vblank_state;
  } else if (vblank_state > 0) {
    --vblank_state;
  }

  if (reg.rendering_enabled && scanline < 240) {
      (*tick_renderer)(*this);
      ++tick_renderer;
  }

  if (++cycle > scanline_end) {

    cycle = 0;
    tick_renderer = renderfuncs.begin();
    scanline_end = 341;

    switch (++scanline) {

      case 261: // Pre-render scanline
        scanline = -1;
        //loopy_w ^= 1; // FIXME: Does the latch get reset here?
        vblank_state = -5;
        break;

      case 241: { // Frame release
        input->tick();
        video->set_buffer(framebuffer);

        clock_frame();
        // TODO: delay?

        vblank_state = 2;
        break;
      }

      default:
        break;

    }
  }

}

void PPU::regw_control(uint8_t value) {
  reg.PPUCTRL = value; 
  scroll.base_nta = reg.base_nta; 
}

void PPU::regw_mask(uint8_t value) {
  reg.PPUMASK = value;
}

void PPU::regw_OAM_address(uint8_t value) {
  reg.OAMADDR = value;
}

void PPU::regw_OAM_data(uint8_t value) {
  OAM[reg.OAMADDR++] = value;
}

void PPU::regw_scroll(uint8_t value) {
  if (loopy_w) {
    scroll.yfine = value & 7;
    scroll.ycoarse = value >> 3;
  } else {
    scroll.xscroll = value;
  }
  loopy_w ^= 1;
}

void PPU::regw_address(uint8_t value) {
  if (loopy_w) {
    scroll.vaddr_lo = value;
    vram.raw = (unsigned)scroll.raw;
  } else {
    scroll.vaddr_hi = value & 0x3f;
  }
  loopy_w ^= 1;
}

void PPU::regw_data(uint8_t value) {
  write(value, vram.raw);
  vram.raw = vram.raw + (bool(reg.vramincr) * 31 + 1);
}

uint8_t PPU::regr_status() {
  uint8_t result { reg.PPUSTATUS };
  reg.vblanking = false;
  loopy_w = false;
  return result;
}

uint8_t PPU::regr_OAM_data() {
  return OAM[reg.OAMADDR] & (reg.OAM_data == 2 ? 0xE3 : 0xFF);
}

uint8_t PPU::regr_data() {
  uint8_t result = read_buffer;
  read_buffer = read(vram.raw, true);
  vram.raw = vram.raw + (!!reg.vramincr * 31 + 1);
  return result;
}

PPU::PPU(IBus *bus, IROM *rom, IInputDevice *input, IVideoDevice *video)
  : bus(bus)
  , rom(rom)
  , input(input)
  , video(video)
  , tick_renderer(renderfuncs.begin())
{
    reg.PPUSTATUS = 0x80;
}


