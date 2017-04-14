#ifndef NES_H
#define NES_H

#include "bus.h"

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

private:
    using clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clock>;

    double _rate{ 1.0 };
    std::unique_ptr<IVideoDevice> video;
    std::unique_ptr<IAudioDevice> audio;
    std::array<std::unique_ptr<IController>, 2> controller;
    std::vector<std::shared_ptr<IInputDevice>> input;
    std::unique_ptr<IROM> rom;
    std::unique_ptr<IPPU> ppu;
    std::unique_ptr<IAPU> apu;
    std::unique_ptr<ICPU> cpu;
    
    time_point _last_frame;
    size_t _frame_counter;
};

#endif
