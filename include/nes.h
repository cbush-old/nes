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
    IROM& rom;
    IPPU *ppu;
    IAPU *apu;
    ICPU *cpu;

  public:
    NES(IROM& rom, std::istream& script);
    virtual ~NES();

  public:
    void run();

  public:
    void pull_NMI() override;
    void pull_IRQ() override;
    void on_frame() override;
    void on_cpu_tick() override;

  public:
    double get_rate() const override;
    void set_rate(double) override;

  private:
    double _rate { 1.0 };

};

#endif
