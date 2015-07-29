#include "nes.h"

#include "rom.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"

#include "controller_std.h"
#include "audio_sdl.h"
#include "audio_sox_pipe.h"
#include "video_sdl.h"
#include "video_tty.h"
#include "video_autosnapshot.h"
#include "input_sdl.h"
#include "input_script.h"
#include "input_script_recorder.h"
#include "log.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <sys/time.h>

class NoAudioDevice : public IAudioDevice {
  public:
    NoAudioDevice(){}
    ~NoAudioDevice(){}

  public:
    void put_sample(int16_t) override {}

};

NES::NES(const char *rom_path, std::istream& script)
    : video (
        new SDLVideoDevice()
        // new TTYVideoDevice()
        // new AutoSnapshotVideoDevice(rom_path, 4)
    )
    , audio (
        new SDLAudioDevice(this)
        // new NoAudioDevice()
    )
    , controller {
        new Std_controller(),
        new Std_controller(),
    }
    , input {
        new SDLInputDevice(*this, *controller[0]),
        // new ScriptInputDevice(*controller[0], script),
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

uint8_t NES::cpu_read(uint16_t addr) const {
    return cpu->read(addr);
}

void NES::pull_NMI() {
    cpu->pull_NMI();
}

void NES::pull_IRQ() {
    cpu->pull_IRQ();
}

void NES::release_IRQ() {
    if (cpu) {
        cpu->release_IRQ();
    }
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

    ((CPU*)cpu)->set_observer16((SDLVideoDevice*)video);
    ((CPU*)cpu)->set_observer((SDLVideoDevice*)video);

    std::thread t0 { [&] {
        cpu->run();
    }};

    try {

        for (;;) {

            _semaphore[0].wait();

#ifdef NES_CONTROLS_DELAY
            timeval t;
            gettimeofday(&t, NULL);
            long tock = ((unsigned long long)t.tv_sec * 1000000) + t.tv_usec;
#endif

            video->on_frame();

            for (auto& i : input) {
                i->tick();
            }

#ifdef NES_CONTROLS_DELAY
            gettimeofday(&t, NULL);
            long tick = ((unsigned long long)t.tv_sec * 1000000) + t.tv_usec;

            std::this_thread::sleep_for(std::chrono::microseconds(
                std::max(0l, 1000 - (tick - tock))
            ));
#endif

            _semaphore[1].signal();
        }

    } catch(std::runtime_error const& e) {
        std::cout << "stop" << std::endl;
        cpu->stop();
        _semaphore[1].signal();
    }

    t0.join();
}
