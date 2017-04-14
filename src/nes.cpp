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

class NoAudioDevice : public IAudioDevice
{
public:
    NoAudioDevice() {}
    ~NoAudioDevice() {}

    void put_sample(int16_t) override {}
};

class NoVideoDevice : public IVideoDevice
{
public:
    NoVideoDevice() {}
    ~NoVideoDevice() {}

    virtual void on_frame() override {}
    virtual void put_pixel(uint8_t x, uint8_t y, PaletteIndex i) override {}
};

NES::NES(const char *rom_path, std::istream &script)
    : video(new SDLVideoDevice)
    //: video(new NoVideoDevice)
    , audio(new SDLAudioDevice(this))
    //, audio(new NoAudioDevice)
    , controller{
        std::unique_ptr<IController>{new Std_controller()},
        std::unique_ptr<IController>{new Std_controller()},
    }
    , input
    {
#if SCRIPT_INPUT
        std::shared_ptr<IInputDevice>{new ScriptInputDevice(*controller[0], script)},
#else
        std::shared_ptr<IInputDevice>{new SDLInputDevice(*this, *controller[0])},
#endif
    // new ScriptRecorder(*controller[0]),
    }
    , rom(load_ROM(this, rom_path))
    , ppu(new PPU(this, rom.get(), video.get()))
    , apu(new APU(this, audio.get()))
    , cpu(new CPU(this, apu.get(), ppu.get(), rom.get(), controller[0].get(), controller[1].get()))
    , _last_second(clock::now())
    , _frame_counter(0)
{
}

NES::~NES()
{}

uint8_t NES::cpu_read(uint16_t addr) const
{
    return cpu->read(addr);
}

void NES::pull_NMI()
{
    cpu->pull_NMI();
}

void NES::pull_IRQ()
{
    cpu->pull_IRQ();
}

void NES::release_IRQ()
{
    if (cpu)
    {
        cpu->release_IRQ();
    }
}

using Clock = std::chrono::high_resolution_clock;
using time_point = Clock::time_point;

time_point tick{ Clock::now() };

void NES::on_frame()
{
    video->on_frame();

    for (auto &i : input)
    {
        i->tick();
    }
    
    ++_frame_counter;
    auto dt = clock::now() - _last_second;
    if (dt >= std::chrono::seconds(1))
    {
        _last_second = clock::now();
        if (_last_fps != _frame_counter)
        {
            std::printf("fps: %lu\n", _frame_counter);
            _last_fps = _frame_counter;
        }
        _frame_counter = 0;
    }
}

void NES::on_cpu_tick()
{
    apu->tick();
    ppu->tick();
    ppu->tick();
    ppu->tick();
}

void NES::set_rate(double rate)
{
    _rate = rate;
}

double NES::get_rate() const
{
    return _rate;
}

void NES::run()
{
    try
    {
        for (;;)
        {
            cpu->run();
        }
    }
    catch (std::runtime_error const &e)
    {
        std::cout << "stop" << std::endl;
        cpu->stop();
    }
}
