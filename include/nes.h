#ifndef NES_H
#define NES_H

#include "bus.h"
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

  public:
    double get_rate() const override;
    void set_rate(double) override;

  public:
    void signal(AsyncComponent const*) override;

  private:
    double _rate { 1.0 };
    uint8_t _ppu_ticks { 0 };
    std::mutex _mutex;
    semaphore _semaphore;
};

#endif
