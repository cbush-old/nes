#ifndef BUS_H
#define BUS_H

#include <string>
#include <vector>

struct State {

  uint8_t P, A, X, Y, SP;
  uint16_t PC;
  int result_cycle;

  std::vector<uint8_t> cpu_memory, ppu_memory, palette;

  uint32_t ppu_reg, scroll, vram;
  
  int read_buffer, vblank_state;
  bool loopy_w, NMI_pulled;
  
  State():
    cpu_memory(0x800, 0xff),
    ppu_memory(0x800),
    palette(0x20)
    {}

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
 * @brief Interface of an onboard component
 **/
class IComponent : public IWriteableComponent {
  public:
    virtual ~IComponent(){}

  public:
    /**
     * @brief read a memory location
     * @note may have non-const side-effects depending on the component
     * @param addr the address to read
     **/
    virtual uint8_t read(uint16_t addr = 0) const =0;

  public:
    /**
     * @brief advance the component's internal clock
     **/
    virtual void tick() =0;

    /**
     * @brief retrieve the current state of the component
     * @return the current state object of the component
     **/
    virtual State const& get_state() const =0;

    /**
     * @brief set the current state of the component
     * @param state the new state
     **/
    virtual void set_state(State const& state) =0;

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


using Raster = std::vector<uint32_t>;


/**
 * Interface for a video output device
 **/
class IVideoDevice {
  public:
    virtual ~IVideoDevice(){}

  public:
    /**
     * @brief set the video buffer to the given raster
     * @param raster the pixel data to set
     **/
    virtual void set_buffer(Raster const& raster) =0;

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

    /**
     * @brief retrieve a reference to a location in the vbank
     * @param addr the address to look up
     * @return a reference to the vbank location
     **/
    virtual uint8_t& getvbankref(uint16_t addr) =0;

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
class ICPU : public IBus, public IComponent {
  public:
    virtual ~ICPU(){}

  public:
    virtual void run() =0;

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
    IComponent *ppu;
    IComponent *apu;
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
