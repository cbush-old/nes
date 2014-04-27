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
    IInputDevice *input;
    IVideoDevice *video;

  public:
    PPU(IBus *bus, IROM *rom, IInputDevice *input, IVideoDevice *video);

  public:
    /**
     * @brief advance the PPU by one step.
     **/
    void tick();

    /**
     * @brief read from a specific register
     * @param index the index, 0-7, of the register to read
     * @note reading certain registers has consequences to the internal state.
     **/
    uint8_t read(uint8_t index) const;

    /**
     * @brief write a value to a register
     * @param what the value to write
     * @param where index (0-7) of the register
     **/
    void write(uint8_t what, uint8_t where);

  private:
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

    mutable bool loopy_w { false };
    bool NMI_pulled { false };

    int
      cycle { 0 },
      scanline { 241 },
      scanline_end { 341 },
      vblank_state { 0 };

    mutable int read_buffer { 0 };

    ObjectAttributeMemory OAM;
    Palette palette;
    Framebuffer framebuffer;

    unsigned pat_addr, sprinpos, sproutpos, sprrenpos, sprtmp;
    uint16_t tileattr, tilepat, ioaddr;
    uint32_t bg_shift_pat, bg_shift_attr;

    struct { 
      uint8_t sprindex, y, index, attr, x; 
      uint16_t pattern; 
    } OAM2[9], OAM3[9];

    template<int, char, bool, bool, bool>
    friend void render(PPU&);

    std::array<std::function<void(PPU&)>, 342>::const_iterator tick_renderer;

    void render_pixel();

  protected: // Register write
    /**
     * @brief write to the ppu control register
     * @param value the value to write
     *
     * This register controls various aspects of the PPU operation:
     * - base nametable address (can be $2000, $2400, $2800 or $2C00)
     * - vram address increment per CPU read/write of PPUDATA (1 or 32)
     * - sprite pattern table address for 8x8 sprites ($0000 or $1000)
     * - background pattern table address ($0000 or $1000)
     * - sprite size (8x8 or 8x16)
     * - PPU master/slave select
     * - whether to generate an NMI at the start of the vertical blanking interval
     * 
     * See http://wiki.nesdev.com/w/index.php/PPU_registers#Controller_.28.242000.29_.3E_write for details.
     **/
    void regw_control(uint8_t value);

    /**
     * @brief write to the ppu mask register
     * @param value the value to write
     *
     * This register can enable or disable certain rendering options:
     * - grayscale
     * - show background in leftmost 8 pixels of screen
     * - show sprites in leftmost 8 pixels of screen
     * - show background
     * - show sprites
     * - intensify reds
     * - intensify greens
     * - intensify blues
     **/
    void regw_mask(uint8_t value);

    /**
     * @brief set the object attribute memory address
     * @param value new OAM address
     **/
    void regw_OAM_address(uint8_t value);

    /**
     * @brief write OAM data, incrementing the OAM address
     * @value the value to write
     *
     * OAM address is incremented after the write.
     **/
    void regw_OAM_data(uint8_t value);

    /**
     * @brief set the scroll position, i.e. where in the nametable to start rendering
     * @param value the address within the nametable that should be drawn at the top left corner: x and y on alternate writes.
     **/
    void regw_scroll(uint8_t value);

    /**
     * @brief set the vram address
     * @param value 1/2 of the 2-byte VRAM address. Upper byte on first write, lower byte on second.
     *
     * An internal latch that determines whether to write to the upper or lower byte is toggled upon write.
     * The latch is reset by a read to $2002 (status register).
     **/
    void regw_address(uint8_t value);

    /**
     * @brief write to the memory at the current vram address
     * @param the value to write
     * 
     * After read/write, the vram address is incremented by either 1 or 32 (as set in the ppu control register).
     **/
    void regw_data(uint8_t value);

  protected: // Register read
    /**
     * @brief read the status register
     * @return the state of the ppu
     *
     * This register holds various state information:
     * - sprite overflow
     * - sprite 0 hit
     * - whether vertical blank has started
     *
     * Reading has certain side effects:
     * - clears the vblank started flag
     * - resets the address latch of the scroll and address registers
     *
     * @note reading this register at exact start of vblank returns 0, but still clears the latch.
     **/
    uint8_t regr_status() const;

    /**
     * @brief read the object attribute memory data
     * @return the current value pointed to by the oam address
     * @note reads during vblank do not increment the address.
     **/
    uint8_t regr_OAM_data() const;

    /**
     * @brief read the value pointed to by the vram address
     * @return the value pointed to by the vram address
     * @note reading this register increments the vram address and may update an internal buffer
     * 
     * Reading non-palette memory area (i.e. below $3f00) returns the contents of an internal buffer,
     * which is then filled with the data at the vram address prior to increment.
     * 
     * When reading palette data, the buffer is filled with the name table data 
     * that would be accessible if the name table mirrors extended up to $3fff.
     **/
    uint8_t regr_data() const;

  protected:
    /**
     * @brief (internal) write to vram at the current vram address
     * @param value the value to write
     **/
    void write_vram(uint8_t value);

    /**
     * @brief (internal) read from vram at the current vram address
     * @return the value at the current vram address
     **/
    uint8_t read_vram(uint16_t addr) const;

};


#endif
