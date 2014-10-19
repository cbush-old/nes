#ifndef NES_H
#define NES_H

#include "bus.h"
#include "semaphore.h"
#include <vector>

/**
 * @brief a basic system
 **/
class NES : public IBus {
  protected:
    IVideoDevice *video;
    IAudioDevice *audio;
    IController *controller[2];
    std::vector<IInputDevice*> input;
    IROM *rom;
    IPPU *ppu;
    IAPU *apu;
    ICPU *cpu;

  public:
    NES(const char *rom_path, std::istream& script);
    virtual ~NES();

  public:
    void run();

  public:
    void pull_NMI() override;
    void pull_IRQ() override;
    void on_frame() override;
    void on_cpu_tick() override;
    uint8_t cpu_read(uint16_t) const override;

  public:
    double get_rate() const override;
    void set_rate(double) override;

  private:
    double _rate { 1.0 };
    semaphore _semaphore[2];
};

#endif
