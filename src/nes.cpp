#include "nes.h"

#include "rom.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"

#include "controller_std.h"
#include "audio_sdl.h"
#include "audio_sox_pipe.h"
#include "video_sdl.h"
#include "input_sdl.h"
#include "input_script.h"
#include "input_script_recorder.h"

#include <iostream>

class NoAudioDevice : public IAudioDevice {
  public:
    NoAudioDevice(){}
    ~NoAudioDevice(){}

  public:
    void put_sample(int16_t) override {}

};

NES::NES(IROM& rom, std::istream& script)
    : video (new SDLVideoDevice())
    , audio (
        new SDLAudioDevice(this)
    )
    , controller {
        new Std_controller(),
        new Std_controller(),
    }
    , input {
        new SDLInputDevice(*this, *controller[0]),
        new ScriptInputDevice(*controller[0], script),
        // new ScriptRecorder(*controller[0]),
    }
    , rom (rom)
    , ppu (new PPU(this, &rom, video))
    , apu (new APU(this, audio))
    , cpu (new CPU(this, apu, ppu, &rom, controller[0], controller[1]))
    {}

NES::~NES() {
    delete apu;
    delete cpu;
    delete ppu;
    delete controller[0];
    delete controller[1];
    delete video;
    delete audio;
    for (auto& i : input) {
        delete i;
    }
}

void NES::pull_NMI() {
    cpu->pull_NMI();
}

void NES::pull_IRQ() {
    cpu->pull_IRQ();
}

using Clock = std::chrono::high_resolution_clock;
using time_point = Clock::time_point;

time_point tick { Clock::now() };

void NES::on_frame() {

    for (auto& i : input) {
        i->tick();
    }

    int s = (int)((1.0 / (60.0 * _rate) * 1000.0));
    std::chrono::milliseconds sleep (s);

    time_point tock (Clock::now());
    tock -= tick.time_since_epoch();
    sleep -= std::chrono::time_point_cast<std::chrono::milliseconds>(tock).time_since_epoch();

    if (sleep < std::chrono::milliseconds(0)) {
        sleep = std::chrono::milliseconds(0);
    }

    std::this_thread::sleep_for(sleep);
    tick = Clock::now();

}

void NES::on_cpu_tick() {
    apu->tick();
    ppu->tick();
    ppu->tick();
    ppu->tick();
}

void NES::set_rate(double rate) {
    _rate = rate;
}

double NES::get_rate() const {
    return _rate;
}

void NES::run() {
    cpu->run();
}
