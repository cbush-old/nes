#ifndef NES_H
#define NES_H

#include "apu.h"
#include "bus.h"
#include "cpu.h"
#include "inline_polymorph.h"
#include "ppu.h"
#include "rom.h"

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
    NES(NES const &) = delete;
    NES &operator=(NES const &) = delete;
    virtual ~NES();

public:
    void run();

    virtual void pull_NMI() override;
    virtual void pull_IRQ() override;
    virtual void release_IRQ() override;
    virtual void on_frame() override;
    virtual void on_cpu_tick() override;

    virtual uint8_t read_cpu(uint16_t) override;
    virtual double get_rate() const override;
    virtual void set_rate(double) override;

    virtual void rewind(bool on) override;

    virtual void put_pixel(uint8_t x, uint8_t y, PaletteIndex i) override;

    virtual uint8_t read_apu() override;
    virtual uint8_t read_chr(uint16_t addr) override;
    virtual uint8_t read_nt(uint16_t addr) override;
    virtual uint8_t read_prg(uint16_t addr) override;
    virtual uint8_t read_ppu(uint16_t addr) override;
    virtual void write_apu(uint8_t value, uint16_t addr) override;
    virtual void write_chr(uint8_t value, uint16_t addr) override;
    virtual void write_nt(uint8_t value, uint16_t addr) override;
    virtual void write_ppu(uint8_t value, uint16_t addr) override;
    virtual void write_prg(uint8_t value, uint16_t addr) override;

private:
    using clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clock>;

    double _rate{ 1.0 };
    std::unique_ptr<IVideoDevice> _video;
    std::unique_ptr<IAudioDevice> _audio;
    std::array<std::unique_ptr<IController>, 2> _controller;
    std::vector<std::shared_ptr<IInputDevice>> _input;
    InlinePolymorph<ROM> _rom;
    PPU ppu;
    APU apu;
    CPU cpu;
    
    std::vector<CPU> _cpu_states;
    std::vector<PPU> _ppu_states;
    std::vector<InlinePolymorph<ROM>> _rom_states;

    time_point _last_frame;
    time_point _last_second;
    size_t _frame_counter;
    size_t _last_fps;
    
    bool _rewinding;
};

#endif
