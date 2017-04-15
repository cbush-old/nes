#ifndef NES_H
#define NES_H

#include "apu.h"
#include "bus.h"
#include "circular_buffer.h"
#include "cpu.h"
#include "ppu.h"

#include <chrono>
#include <memory>
#include <vector>

/**
 * @brief a basic system
 **/
class NES : public IBus
{
public:
    NES(const char *rom_path, std::istream &script);
    virtual ~NES();

public:
    void run();

    virtual void pull_NMI() override;
    virtual void pull_IRQ() override;
    virtual void release_IRQ() override;
    virtual void on_frame() override;
    virtual void on_cpu_tick() override;
    virtual uint8_t cpu_read(uint16_t) const override;
    virtual double get_rate() const override;
    virtual void set_rate(double) override;

    virtual void rewind(bool on) override;

    virtual uint8_t read_chr(uint16_t addr) const override;
    virtual uint8_t read_nt(uint16_t addr) const override;
    virtual void write_chr(uint8_t value, uint16_t addr) override;
    virtual void write_nt(uint8_t value, uint16_t addr) override;
    virtual void put_pixel(uint8_t x, uint8_t y, PaletteIndex i) override;

private:
    using clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clock>;

    double _rate{ 1.0 };
    std::unique_ptr<IVideoDevice> _video;
    std::unique_ptr<IAudioDevice> _audio;
    std::array<std::unique_ptr<IController>, 2> _controller;
    std::vector<std::shared_ptr<IInputDevice>> _input;
    std::unique_ptr<IROM> _rom;
    PPU ppu;
    APU apu;
    CPU cpu;
    
    std::vector<CPU> _cpu_states;
    std::vector<PPU> _ppu_states;

    time_point _last_frame;
    time_point _last_second;
    size_t _frame_counter;
    size_t _last_fps;
    
    bool _rewinding;
};

#endif
