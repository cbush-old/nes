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

NES::NES(const char *rom_path, std::istream& script)
    : video (new SDLVideoDevice())
    , audio (
        //new SDLAudioDevice(this)
        new NoAudioDevice()
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
    , rom (load_ROM(this, rom_path))
    , ppu (new PPU(this, rom, video))
    , apu (new APU(this, audio))
    , cpu (new CPU(this, apu, ppu, rom, controller[0], controller[1]))
    {}

NES::~NES() {
    delete apu;
    delete cpu;
    delete ppu;
    unload_ROM(rom);
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
    _semaphore[0].signal();
    _semaphore[1].wait();
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

    std::thread t0 { [&] {
        cpu->run();
    }};

    t0.detach();

    for (;;) {

        _semaphore[0].wait();

        video->on_frame();

        try {
            for (auto& i : input) {
                i->tick();
            }
        } catch(int) {
            std::cout << "stop" << std::endl;
            cpu->stop();
            break;
        }

        _semaphore[1].signal();
    }

}
