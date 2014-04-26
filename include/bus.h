#ifndef BUS_H
#define BUS_H

#include <string>
#include <vector>
#include <array>

using Framebuffer = std::array<uint32_t, 256 * 240>;

struct State {
};

/**
 * @brief Interface of writeable component
 **/
class IWriteableComponent {
  public:
    ~IWriteableComponent() {}

  public:
    /**
     * @brief write a value to a memory location
     * @param what the 8-bit value to write
     * @param where the address where to write the value
     **/
    virtual void write(uint8_t what, uint16_t where) =0;

};


/**
 * @brief Interface of a picture processing unit
 **/
class IPPU {
  public:
    virtual ~IPPU(){}

  public:
    /**
     * @brief read a register
     * @note may have non-const side-effects depending on the register
     * @param index the register (0-7) to read
     **/
    virtual uint8_t read(uint8_t index) const =0;

    /**
     * @brief write a value to a register
     * @param value the value to write
     * @param index the register (0-7) to write to
     **/
    virtual void write(uint8_t value, uint8_t index) =0;

    /**
     * @brief advance the component's internal clock
     **/
    virtual void tick() =0;

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

};


/**
 * Interface for an audio output device
 **/
class IAudioDevice {
  public:
    virtual ~IAudioDevice(){}
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
class IROM : public IWriteableComponent {
  public:
    virtual ~IROM(){}

  public:
    /**
     * @brief get a reference to the value at a memory location
     * @param addr the address where the reference should be found
     * @return a reference to the value at the memory location
     **/
    virtual uint8_t& getmemref(uint16_t addr) =0;

    /**
     * @brief retrieve a reference to the value at a nametable memory location
     * @param table the index of the nametable
     * @param addr the address within the nametable
     * @return a reference to the value at the nametable memory location
     **/
    virtual uint8_t& getntref(uint8_t table, uint16_t addr) =0;
    virtual uint8_t const& getntref(uint8_t table, uint16_t addr) const =0;
    virtual void write_nt(uint8_t value, uint8_t table, uint16_t addr) =0;

    /**
     * @brief retrieve a reference to a location in the vbank
     * @param addr the address to look up
     * @return a reference to the vbank location
     **/
    virtual uint8_t& getvbankref(uint16_t addr) =0;
    virtual uint8_t const& getvbankref(uint16_t addr) const =0;

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
    virtual void reset_IRQ() =0;

};


/**
 * @brief Interface for the CPU
 **/
class ICPU {
  public:
    virtual ~ICPU(){}

  public:
    virtual void run() =0;

  public:
    virtual void pull_NMI() =0;
    virtual void pull_IRQ() =0;
    virtual void reset_IRQ() =0;

};


/**
 * @brief basic system
 **/
class NES : public IBus {
  protected:
    IVideoDevice *video;
    IAudioDevice *audio;
    IController *controller[2];
    IInputDevice *input;
    IROM *rom { nullptr };
    IPPU *ppu;
    IAPU *apu;
    ICPU *cpu;

  public:
    NES(std::string const&);
    virtual ~NES();

  public:
    void pull_NMI();
    void pull_IRQ();
    void reset_IRQ();

};

#endif
