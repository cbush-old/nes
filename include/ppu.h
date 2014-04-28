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

#include "bit.h"
#include "bus.h"


class PPU : public IPPU {
  public:
    using ObjectAttributeMemory = std::array<uint8_t, 0x100>;
    using Palette = std::array<uint8_t, 0x20>;

  private:
    IBus *bus;
    IROM *rom;
    IVideoDevice *video;

  public:
    PPU(IBus *bus, IROM *rom, IVideoDevice *video);

  public:
    /**
     * @brief advance the PPU by one step.
     **/
    void tick();

    void set_frameskip(int n);

  private:
    int frameskip { 0 };
    int frameskip_count { 0 };

    mutable union {
      
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
    
    mutable union {
      uint32_t data;

      bit< 3, 16> raw;
      bit< 0,8> xscroll;
        bit< 0,3> xfine;
        bit< 3,5> xcoarse;
      
      bit< 3,8> vaddr_lo;
        bit< 8,5> ycoarse;
      
      bit<11,8> vaddr_hi;
        bit<13,2> base_nta;
          bit<13,1> base_nta_x;
        bit<14,1> base_nta_y;
        bit<15,3> yfine;

    } scroll, vram;

    bool loopy_w { false };

    int cycle { 0 };
    int scanline { 241 };
    int scanline_end { 341 };

    mutable int read_buffer { 0 };

    ObjectAttributeMemory OAM;
    Palette palette;
    Framebuffer framebuffer;

    unsigned pat_addr, sprinpos, sproutpos, sprrenpos, sprtmp;
    uint16_t tileattr, tilepat, ioaddr;
    uint32_t bg_shift_pat, bg_shift_attr;

    struct { 
      uint8_t y;
      uint8_t index;
      union {
        uint8_t attr;
        bit<0, 2, uint8_t> palette;
        bit<5, 1, uint8_t> priority;
        bit<6, 1, uint8_t> flip_horizontally;
        bit<7, 1, uint8_t> flip_vertically;
      };
      uint8_t x;
      uint8_t sprindex;
      uint16_t pattern; 
    } OAM2[8], OAM3[8];

    void render_pixel();

    void release_frame();

  public: // Register read/write
    void regw_control(uint8_t value);
    void regw_mask(uint8_t value);
    void regw_OAM_address(uint8_t value);
    void regw_OAM_data(uint8_t value);
    void regw_scroll(uint8_t value);
    void regw_address(uint8_t value);
    void regw_data(uint8_t value);
    uint8_t regr_status();
    uint8_t regr_OAM_data();
    uint8_t regr_data();

  private: // Render functions
    using Renderf = std::function<void(PPU&)>;
    using Renderf_array = std::array<Renderf, 342>;

    template<int, char, bool, bool, bool>
    void render();

    static const Renderf_array renderfuncs;
    Renderf_array::const_iterator tick_renderer;

  protected:
    /**
     * @brief (internal) write to vram
     * @param value the value to write
     * @param addr target location of the write
     **/
    void write(uint8_t value, uint16_t addr);

    /**
     * @brief (internal) read from vram
     * @param addr the vram address to look up
     * @param no_palette whether to treat the memory space above $3f00 as mirror of nametable below (usually no).
     * @return the value at the given address
     **/
    uint8_t read(uint16_t addr, bool no_palette = false) const;

};


#endif
