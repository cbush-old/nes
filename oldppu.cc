std::vector<uint8_t> ram, plt, oam;    
    
    // registers
    uint8_t
      ctrl, mask, status { 0xa0 }, 
      oamaddr, oamdata, data, NMI_occurred;
    
    // PPUCTRL helpers
    inline uint16_t base_nametable_addr(){ return 0x2000 + (ctrl & 3) * 0x400; }
    inline uint8_t increment_coarse_y(){ return bool(ctrl & 0x04); }
    inline uint16_t sprite_pattern_table_addr(){ return bool(ctrl & 0x08) * 0x1000; }
    inline uint16_t background_pattern_table_addr(){ return bool(ctrl & 0x10) * 0x1000; }
    inline bool sprites_are_8x16(){ return bool(ctrl & 0x20); }
    inline bool ppu_master_mode(){ return bool(ctrl & 0x40); }
    inline bool generate_NMI_at_vertical_blank(){ return bool(ctrl & 0x80); }

    // PPUMASK helpers
    inline bool grayscale_mode(){ return bool(mask & 0x01); }
    inline bool background_clip_off(){ return bool(mask & 0x02); }
    inline bool sprite_clip_off(){ return bool(mask & 0x04); }
    inline bool show_background(){ return bool(mask & 0x08); }
    inline bool show_sprites(){ return bool(mask & 0x10); }
    inline bool intensify_reds(){ return bool(mask & 0x20); }
    inline bool intensify_greens(){ return bool(mask & 0x40); }
    inline bool intensify_blues(){ return bool(mask & 0x80); }
    inline bool rendering_enabled(){
      return true;
      return show_background() | show_sprites();
    }
    
    int
      scanline { 261 },
      frame { 0 },
      cycle { 340 };
    
    int& dot { cycle };
    
    uint8_t loopy_x; // three-bit
    bool loopy_w;
    uint8_t buffer2007;

    struct loopy_struct {
      struct {
        uint8_t x, y;
      } coarse, nametable_select;
      uint8_t fine_y; 
      
      void inc_x(){
        if(coarse.x == 31){
          coarse.x = 0;
          nametable_select.x ^= 1;
        } else {
          ++coarse.x;
        }
      }
      
      void inc_y(){
        if(fine_y < 7){
          ++fine_y;
        } else {
          fine_y = 0;
          if(coarse.y == 29){
            coarse.y = 0;
            nametable_select.y ^= 1;
          } else if(coarse.y == 31){
            coarse.y = 0;
          } else {
            ++coarse.y;
          }
        }
      }
      
      uint8_t attr_coarse_x(){
        return coarse.x & 7;
      }
      
      uint8_t attr_coarse_y(){
        return ((coarse.x >> 3) | (coarse.y << 2)) & 7;
      }
      
      uint8_t attr_offset(){
        return coarse.y >> 1;
      }
      
      operator uint16_t(){
        return ((int)coarse.x 
          | ((int)coarse.y << 5)
          | ((int)(bool(nametable_select.x)) << 10)
          | ((int)(bool(nametable_select.y)) << 11)
          | ((int)fine_y << 12));
          
      }
      
      loopy_struct& operator=(loopy_struct const& that){
        coarse.x = that.coarse.x;
        coarse.y = that.coarse.y;
        fine_y = that.fine_y;
        nametable_select.x = that.nametable_select.x;
        nametable_select.y = that.nametable_select.y;
        return *this;
      }
    } loopy_v, loopy_t;
    
    uint16_t test_loopy_v;

    uint16_t bg_shiftreg16[2]; // bmp data for two tiles
    uint8_t 
      bg_shiftreg8[2], // background palette attributes
      bg_latch; // background attribute latch
    
    uint8_t 
      spr_shiftreg8[8][2],
      spr_latch[8],
      spr_x_pos[8];
    
    
    uint8_t next_bg_pixel(){
      uint8_t pixel =
        (uint8_t(bg_shiftreg16[0] >> loopy_x) & 1)
        | ((uint8_t(bg_shiftreg16[1] >> loopy_x) & 1) << 1)
        | (((bg_shiftreg8[0] >> loopy_x) & 1) << 2)
        | (((bg_shiftreg8[1] >> loopy_x) & 1) << 3);
      
      bg_shiftreg16[0] >>= 1;
      bg_shiftreg16[1] >>= 1;
      bg_shiftreg8[0] >>= 1;
      bg_shiftreg8[1] >>= 1;
    
      return pixel;
      
    }
    
    uint8_t load_bg_pixel(){
      
    
    }
    
    inline void dec_sprite_x(){
      for(int i = 0; i < 8; ++i){
        if(!--spr_x_pos[i]){
          spr_shiftreg8[i][0] >>= 1;
          spr_shiftreg8[i][1] >>= 1;
        }
      }
    }
    
    uint16_t ntbyte { 0 }, atbyte { 0 };
    
    std::vector<std::function<void()>> accessf {
      /* 0-1 */ [&]{  // Nametable byte
        ntbyte
          = base_nametable_addr() 
          + 0x10 * loopy_v.coarse.y
          + loopy_v.coarse.x;
        
      },
      /* 2-3 */ [&]{  // Attribute byte
        atbyte
          = base_nametable_addr()
          + 0x23c0
          + 8 * loopy_v.attr_coarse_y()
          + loopy_v.attr_coarse_x();
      },
      /* 4-5 */ [&]{  // Tile bitmap low
        bg_shiftreg16[0] &= 0x00ff;
        bg_shiftreg16[0] |= read(ntbyte) << 8;
      },
      /* 6-7 */ [&]{  // Tile bitmap high
        bg_shiftreg16[1] &= 0x00ff;
        bg_shiftreg16[1] |= read(ntbyte + 8) << 8;
      }
    
    };
    
    PPU& tick(){
      
      if(++cycle > 341){
        cycle = 0;
        
        if(++scanline > 261){
          scanline = 0;
        }
      }
      
      if(scanline < 240){
        if(cycle & 1){
          if(cycle < 257){
            accessf[(cycle&7)/2]();
            // sprite evaluation
          } else if(cycle < 321){
            accessf[(cycle&7)/2]();
            // load x pos and attr for sprites from 2nd OAM
          } else if(cycle < 337){
            accessf[(cycle&7)/2]();
          } else if(cycle < 341){
            accessf[0](); // dummy nametable byte?
          }
        } 
      } else if(scanline == 241 && cycle == 1){
        set_vblank_flag();
      }
      
      dec_sprite_x();
      
      if(rendering_enabled())
        render();
      
      
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
    
    std::vector<regrf> regr {
      /* 0 */ bad_read, 
      /* 1 */ bad_read,
      /* 2 */ [&] {
        // account for race condition
        uint8_t res = (unsigned)(scanline * 341 + cycle - 82180) < 2 ? 
          status & 0x7F : 
          status|(NMI_occurred << 7);
        
        // clear the toggle, NMI occurred and vblank flag
        loopy_w = false;
        NMI_occurred = false;
        status = res & 0x7f;
        return res;
        
      },
      /* 3 */ bad_read,
      /* 4 */ [&] { return oam[oamaddr]; },
      /* 5 */ bad_read, 
      /* 6 */ bad_read,
      /* 7 */ [&] {
        if(increment_coarse_y()){
          loopy_v.inc_y();
        } else {
          loopy_v.inc_x();
        }
        return data;
      }
    };
    
    std::vector<regwf> regw {
      /* Reg# */
      /* 0 */ [&](uint8_t value){
        loopy_t.nametable_select.x = value & 1;
        loopy_t.nametable_select.y = bool(value & 2);
        //NMI_output = (bool)(value&0x80);
        ctrl = value;
      },
      
      /* 1 */ [&](uint8_t value){ mask = value; },
      /* 2 */ bad_write,
      /* 3 */ [&](uint8_t value){ oamaddr = value; },
      /* 4 */ [&](uint8_t value){ oam[oamaddr++] = value; },
      /* 5 */ [&](uint8_t value){
        if(!loopy_w){
          loopy_t.coarse.x = value >> 3;
          loopy_x = value & 7;
          loopy_w = 1;
        } else {
          loopy_t.coarse.y = value >> 3;
          loopy_t.fine_y = value & 7;
          loopy_w = 0;
        }
      },
      /* 6 */ [&](uint8_t value){
        if(!loopy_w){
          loopy_t.fine_y = (value >> 3) & 7;
          loopy_t.nametable_select.x = bool((value >> 1) & 1);
          loopy_t.nametable_select.y = bool((value >> 1) & 2);
          loopy_t.coarse.y 
            = (loopy_t.coarse.y & 0xf) | ((value & 1) << 5);
          loopy_w = 1;
        } else {
          loopy_t.coarse.x = value & 0xf;
          loopy_t.coarse.y 
            = (loopy_t.coarse.y & 0x18) | (value >> 5);
          loopy_v = loopy_t;
          loopy_w = 0;
        }        
      },
      /* 7 */ [&](uint8_t value){
        write(value, loopy_v);
        if(increment_coarse_y()){
          loopy_v.inc_y();
        } else {
          loopy_v.inc_x();
        }
      },
    };
    
    void inc_loopy_x(){
      if(loopy_x == 7){
        loopy_x = 0;
        loopy_v.inc_x();
      } else {
        ++loopy_x;
      }
    }
    

    void clear_vblank_flag(){
      status &=~ 0x80;
    }
    
    void set_vblank_flag(){
      status |= 0x80;
      bus::io().swap();
    }
    
    void render(){
      
      if(dot == 256){
        loopy_v.inc_y();
      } else if(dot == 257){
        loopy_v.coarse.x = loopy_t.coarse.x;
        loopy_v.nametable_select.x = loopy_t.nametable_select.x;
      } else if(279 < dot && dot < 305){
        loopy_v.coarse.y = loopy_t.coarse.y;
        loopy_v.nametable_select.y = loopy_t.nametable_select.y;
        loopy_v.fine_y = loopy_t.fine_y;
      } else if(327 < dot || dot < 256){
        inc_loopy_x();
      }
      
      //cout << bg_shiftreg16[0] << '\n';
      
      uint8_t np = next_bg_pixel();
      
      cout << (int)np << '\n';
      
      bus::io().put_pixel(
        dot, scanline, 
        ((bg_shiftreg16[0] >> loopy_x) & 1) * 100,
        ((bg_shiftreg16[1] >> loopy_x) & 1) * 100,
        int(64 + sin((double)dot) * 64.0)
      );
    
    }
    
    
  public:
    PPU(): ram(0x800), plt(0x20), oam(0x100) {}
