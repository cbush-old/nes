
// sexy bisqwit
template<size_t position, size_t length, typename T=uint32_t>
struct bit {
  T value;
  constexpr unsigned mask(){ 
    return ((1u << length)-1u);
  };
  template<typename T2>
  bit& operator= (T2 that){
    value &= mask();
    value |= (that << position) & mask();
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
      uint32_t raw;
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
    } loopy_v, loopy_t;
    
    bool loopy_w;
    
    int
      cycle { 0 }, 
      scanline { 241 },
      scanline_end { 341 },
      read_buffer { 0 },
      vblank_state { 0 };
      
    uint8_t& mmap(uint16_t addr){
      addr &= 0x3fff;
      if(addr < 0x2000) return bus::rom()[addr];
      if(addr < 0x3f00) return bus::rom()[addr];
      return palette[addr&3 == 0 ? addr & 0xf : addr & 0x1f];
    }
    
    void render(){
    
    }
    
    void render_pixel(){
    
    }
    
    PPU& tick(){
      
      switch(vblank_state){
        case -5: reg.PPUSTATUS = 0; break;
        case 2: reg.vblanking = true; break;
        case 0: 
          if(reg.vblanking && reg.NMI_enabled) bus::pull_NMI();
          break;
      }
      if(vblank_state != 0){
        vblank_state += (vblank_state < 0) * 2 - 1;
      }
      
      if(scanline < 240){
        if(reg.rendering_enabled) render();
        if(scanline >= 0 && cycle < 256) render_pixel();
      }
      
      if(++cycle > scanline_end){
        scanline_end = 341;
        switch(++scanline){
          case 261:
            scanline = -1;
            loopy_w ^= 1;
            vblank_state = -5;
            break;
          case 241:
            // events
            vblank_state = 2;
            break;
        }
      }
      return *this;
    }
    
    using regrf = std::function<uint8_t()>;
    using regwf = std::function<void(uint8_t)>;
    
    #define BAD_READ [&]{ return 0; }
    #define BAD_WRITE [&](uint8_t){ return; }
    
    std::vector<regrf> regr {
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
        read_buffer = mmap(loopy_t.raw);
        loopy_t.raw = loopy_t.raw + (bool(reg.vramincr) * 31 + 1);
        return result;
      }
    };
    
    std::vector<regwf> regw {
      /* 0 */ [&](uint8_t x) { reg.PPUCTRL = x, loopy_v.base_nta = reg.base_nta; },
      /* 1 */ [&](uint8_t x) { reg.PPUMASK = x; },
      /* 2 */ BAD_WRITE,
      /* 3 */ [&](uint8_t x) { reg.OAMADDR = x; },
      /* 4 */ [&](uint8_t x) { OAM[reg.OAMADDR++] = x; },
      /* 5 */ [&](uint8_t x) { 
        if(loopy_w){
          loopy_v.yfine = x & 7;
          loopy_v.ycoarse = x >> 3;
        } else {
          loopy_v.xscroll = x;
        }
        loopy_w ^= 1;
      },
      /* 6 */ [&](uint8_t x) {
        if(loopy_w){
          loopy_v.vaddr_lo = x;
          loopy_t.raw = (unsigned)loopy_v.raw;
        } else {
          loopy_v.vaddr_hi = x & 0x3f;
        }
        loopy_w ^= 1;
      },
      /* 7 */ [&](uint8_t x) {
        mmap(loopy_t.raw) = x;
        loopy_t.raw = loopy_t.raw + (bool(reg.vramincr) * 31 + 1);
      }
    };
    
    #undef BAD_READ
    #undef BAD_WRITE
  
};
