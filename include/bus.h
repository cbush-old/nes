#ifndef BUS_H
#define BUS_H

#include <string>
#include <vector>
#include <array>

using Framebuffer = std::array<uint32_t, 256 * 240>;


class State {
};

/**
 * @brief Interface of a picture processing unit
 **/
class IPPU {
  public:
    virtual ~IPPU(){}

  public:
    /**
     * @brief start the ppu engine
     **/
    virtual void start() =0;

    /**
     * @brief notify the ppu that a cpu cycle has passed
     **/
    virtual void on_cpu_tick() =0;


  public: // Register write
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
    virtual void regw_control(uint8_t value) =0;

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
    virtual void regw_mask(uint8_t value) =0;

    /**
     * @brief set the object attribute memory address
     * @param value new OAM address
     **/
    virtual void regw_OAM_address(uint8_t value) =0;

    /**
     * @brief write OAM data, incrementing the OAM address
     * @value the value to write
     *
     * OAM address is incremented after the write.
     **/
    virtual void regw_OAM_data(uint8_t value) =0;

    /**
     * @brief set the scroll position, i.e. where in the nametable to start rendering
     * @param value 1/2 of the 2-byte address within the nametable that should be drawn at the top left corner
     *
     * The internal latch that determines whether to write to the upper or lower byte is toggled upon write.
     * The latch is reset by a read to $2002 (status register).
     **/
    virtual void regw_scroll(uint8_t value) =0;

    /**
     * @brief set the vram address
     * @param value 1/2 of the 2-byte VRAM address. Upper byte on first write, lower byte on second.
     *
     * The internal latch that determines whether to write to the upper or lower byte is toggled upon write.
     * The latch is reset by a read to $2002 (status register).
     **/
    virtual void regw_address(uint8_t value) =0;

    /**
     * @brief write to the memory at the current vram address
     * @param the value to write
     * 
     * After read/write, the vram address is incremented by either 1 or 32 (as set in the ppu control register).
     **/
    virtual void regw_data(uint8_t value) =0;


  public: // Register read
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
    virtual uint8_t regr_status() =0;

    /**
     * @brief read the object attribute memory data
     * @return the current value pointed to by the oam address
     * @note reads during vblank do not increment the address.
     **/
    virtual uint8_t regr_OAM_data() =0;

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
    virtual uint8_t regr_data() =0;

};


/**
 * @brief Interface of an onboard audio processing unit
 **/
class IAPU {
  public:
    virtual ~IAPU(){}

  public:
    /**
     * @brief read the current state of the apu
     * @note has side effects
     **/
    virtual uint8_t read() const =0;

    /**
     * @brief write a value to a register
     * @param value the value to write
     * @param index the register to write to
     **/
    virtual void write(uint8_t value, uint8_t index) =0;

    /**
     * @brief advance the component's internal clock
     **/
    virtual void tick() =0;

};


/**
 * Interface for game controller (i.e., GamePad)
 **/
class IController {
  public:
    virtual ~IController(){}

  public:
    using ButtonState = uint8_t;
    static const ButtonState BUTTON_OFF { 0 }, BUTTON_ON { 0xff };

    enum Button {
      CONTROL_RIGHT = 0,
      CONTROL_LEFT  = 1,
      CONTROL_DOWN  = 2,
      CONTROL_UP    = 3,
      CONTROL_START = 4,
      CONTROL_SELECT = 5,
      CONTROL_B = 6,
      CONTROL_A = 7,
      N_BUTTONS = 8,
    };

  public:
    /**
     * @brief read the current state of the controller
     **/
    virtual ButtonState read() =0;

    /**
     * @brief set the state for a button
     * @param button the button to change
     * @param state the new state (on/off)
     **/
    virtual void set_button_state(Button button, ButtonState state) =0;

    /**
     * @brief get the state for a button (for observers)
     * @param button the button to query
     **/
    virtual ButtonState get_button_state(Button button) =0;

    /**
     * @brief strobe the controller
     **/
    virtual void strobe() =0;

};


/**
 * Interface for a video output device
 **/
class IVideoDevice {
  public:
    virtual ~IVideoDevice(){}

  public:
    /**
     * @brief set the video buffer to the given raster
     * @param buffer the pixel data to set
     **/
    virtual void set_buffer(Framebuffer const& buffer) =0;

    /**
     * @brief alert the video device that a frame is ready
     **/
    virtual void on_frame() =0;

};


/**
 * Interface for an audio output device
 **/
class IAudioDevice {
  public:
    virtual ~IAudioDevice(){}

  public:
    virtual void put_sample(int16_t) =0;
};


/**
 * Interface for an input device
 **/
class IInputDevice {
  public:
    virtual ~IInputDevice(){}

  public:
    /**
     * @brief handle the input at the current frame
     **/
    virtual void tick() =0;

};


/**
 * @brief interface of the gamepak/cartridge/ROM
 **/
class IROM {
  public:
    virtual ~IROM(){}

  public: // CPU-space access
    /**
     * @brief write to the cartridge prg space ($4020 - $ffff in cpu memory space)
     * @param value the value to write
     * @param addr the address to write to (expressed in cpu space)
     *
     * This function expects addr to be expressed in terms of cpu space ($4020 - $ffff)
     **/
    virtual void write_prg(uint8_t value, uint16_t addr) =0;

    /**
     * @brief read cartridge prg space ($4020 - $ffff in CPU memory space)
     * @param addr the address to read
     * @return the value at address
     *
     * This function expects addr to be expressed in terms of CPU space ($4020 - $ffff)
     **/
    virtual uint8_t read_prg(uint16_t addr) const =0;


  public: // PPU-space access
    /**
     * @brief write to the nametable space (PPU space $2000 - $2fff)
     * @param value the value to write
     * @param addr the address (expressed in ppu space) to write to
     *
     * This function expects addr to be expressed in PPU space, in the range of $2000 to $2fff
     *
     * @note the name table memory is physically located on the NES; additional memory 
     *       may be provided on the cartridge. Since the use (e.g. mirroring) and size
     *       of the space is managed by the cart, it is nicer, if slightly less realistic,
     *       to model the memory as part of the cart and not part of the NES.
     **/
    virtual void write_nt(uint8_t value, uint16_t addr) =0;

    /**
     * @brief read the nametable space (PPU space $2000 - $2fff)
     * @param addr the address (expressed in ppu space) to read
     * @return the value at the address
     *
     * This function expects addr to be in the range of $2000 to $2fff
     *
     * @note see @ref write_nt for the reasoning why the nt is a part of the ROM class.
     **/
    virtual uint8_t read_nt(uint16_t addr) const =0;

    /**
     * @brief write to the cartridge chr space ($0000 - $1fff in PPU memory space)
     * @param value the value to write
     * @param addr the address to write to (expressed in ppu space)
     *
     * This function expects addr to be expressed in terms of ppu space ($0000 - $1fff)
     **/
    virtual void write_chr(uint8_t value, uint16_t addr) =0;


    /**
     * @brief read the cartridge chr space ($0000 - $1fff in PPU memory space)
     * @param addr the address to read (expressed in ppu space)
     *
     * This function expects addr to be expressed in terms of ppu space ($0000 - $1fff)
     **/
    virtual uint8_t read_chr(uint16_t addr) const =0;

};



/**
 * @brief interface for a basic system, handling interrupts
 **/
class IBus {
  public:
    virtual ~IBus(){}

  public:
    virtual void pull_NMI() =0;
    virtual void pull_IRQ() =0;

  public: // Events
    /**
     * @brief function to be called on every cpu cycle
     **/
    virtual void on_cpu_tick(){}

    /**
     * @brief function to be called on every frame
     **/
    virtual void on_frame() =0;

    /**
     * @brief function to be called every emulated second
     **/
    virtual void on_second_elapsed(){}

  public: // Emulation settings
    /**
     * @brief get the rate of emulation
     * @return 1.0 for normal speed emulation; higher values = faster emulation
     **/
    virtual double get_rate() const =0;

    /**
     * @brief set the rate of emulation
     * @param rate the rate modifier: 1.0 for normal speed; higher = faster
     **/
    virtual void set_rate(double rate) =0;

};


/**
 * @brief Interface for the CPU
 **/
class ICPU {
  public:
    virtual ~ICPU(){}

  public:
    /**
     * @brief start emulation
     **/
    virtual void run() =0;

  public:
    /**
     * @brief trigger a non-maskable interrupt (nmi) in the cpu
     **/
    virtual void pull_NMI() =0;

    /**
     * @brief signal an interrupt request (irq) to the cpu
     **/
    virtual void pull_IRQ() =0;

};


#endif
