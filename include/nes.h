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
    void pull_NMI();
    void pull_IRQ();
    void on_frame();
    void on_cpu_tick();

};

#endif
