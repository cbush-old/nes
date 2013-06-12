template<int X>
const void render() {
  enum {
    tdm = 0x10ffff & (1u << (X/16)) // When x is 0..255, 320..335
  };
  
  tile_decode_mode = tdm;
  
  switch(X&7){
    case 2: {
      ioaddr = 0x23c0 + 0x400 * vram.base_nta 
        + 8 * vram.ycoarse / 4 + vram.xcoarse / 4; 
      if(tile_decode_mode) break;
    } 
    case 0:
    /* 0 */ {
      ioaddr = 0x2000 + (vram.raw & 0xfff);
      if(X == 0){
        sprinpos = sproutpos = 0;
        if(reg.show_sp)
          reg.OAMADDR = 0;
      }
      
      if(!reg.show_bg) break;
      
      if(X == 304)
        if(scanline == -1)
          vram.raw = (unsigned)scroll.raw;
      
      if(X == 256){
        vram.xcoarse = (unsigned)scroll.xcoarse;
        vram.base_nta_x = (unsigned)scroll.base_nta_x;
        sprrenpos = 0;
      }
      
    } break;
    case 1: {
      if(X==337){
        if(scanline == -1 && loopy_w && reg.show_bg){
          scanline_end = 340;
        }
      }
      
      pat_addr = 0x1000 * reg.bg_addr + 16 * mmap(ioaddr) + vram.yfine;
      
      if(!tile_decode_mode) break;
      
      bg_shift_pat = (bg_shift_pat >> 16) + 0x00010000 * tilepat;
      bg_shift_attr = (bg_shift_attr >> 16) + 0x55550000 * tileattr;
      
    } break;
    
    case 3: {
      if(tile_decode_mode){
        tileattr = 
          (mmap(ioaddr) >> ((vram.xcoarse & 2)
          + 2 * (vram.ycoarse&2))) & 3;
        
        if(!++vram.xcoarse) vram.base_nta_x = 1 - vram.base_nta_x;
        
        if(X==251){
          if(!++vram.yfine && ++vram.ycoarse == 30){
          vram.ycoarse = 0;
          vram.base_nta_y = 1 - vram.base_nta_y;
          }
        }
      
      } else if(sprrenpos < sproutpos){
        auto& o = OAM3[sprrenpos];
        memcpy(&o, &OAM2[sprrenpos], sizeof(o));
        unsigned y = scanline - o.y;
        if(o.attr & 0x80){
          y ^= 7 + reg.sprite_size * 8;
        }
        pat_addr = 0x1000 * (reg.sprite_size ? o.index & 0x01 : reg.sp_addr);
        pat_addr += 0x10 * (reg.sprite_size ? o.index & 0xfe : o.index & 0xff);
        pat_addr += (y&7) + (y&8) * 2;
      }
      
    } break;
    case 5: {
      tilepat = mmap(pat_addr);
    } break;
    case 7: {
      unsigned p = tilepat | (mmap(pat_addr|8) << 8);
      p = (p&0xf00f) | ((p&0x0f00)>>4) | ((p&0x00f0)<<4);
      p = (p&0xc3c3) | ((p&0x3030)>>2) | ((p&0x0c0c)<<2);
      p = (p&0x9999) | ((p&0x4444)>>1) | ((p&0x2222)<<1);
      tilepat = p;
      if(!tile_decode_mode && sprrenpos < sproutpos)
        OAM3[sprrenpos++].pattern = tilepat;
    } break;
  }

  if(X & 1 && X >= 64 && X < 256){
    
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
  
  if(X < 256){
    if(scanline >= 0) render_pixel();
  }
  
}

#define r(n) &PPU::render<n>
#define s(n) r(n),r(n+1),r(n+2),r(n+3)
#define t(n) s(n),s(n+4),s(n+8),s(n+12)
const void(PPU::*renderfs[342])() {
  t(0),t(16),t(32),t(48),t(64),t(80),
  t(96),t(112),t(128),t(144),t(160),t(176),
  t(192),t(208),t(224),t(240),t(256),
  t(272),t(288),t(304),t(320),
  s(336),r(340),r(341)
};
#undef r
#undef s
#undef t
