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

    virtual void put_sample(int16_t sample, size_t hz) override {}
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
    : _video(new SDLVideoDevice)
    , _audio(new SDLAudioDevice(this))
    , _controller{
        std::unique_ptr<IController>{new Std_controller()},
        std::unique_ptr<IController>{new Std_controller()},
    }
    , _input
    {
#if SCRIPT_INPUT
        std::shared_ptr<IInputDevice>{new ScriptInputDevice(*_controller[0], script)},
#else
        std::shared_ptr<IInputDevice>{new SDLInputDevice(*this, *_controller[0])},
#endif
    // new ScriptRecorder(*_controller[0]),
    }
    , _rom(load_ROM(this, rom_path))
    , ppu(this)
    , apu(this, _audio.get())
    , cpu(this, &apu, &ppu, _rom.get(), _controller[0].get(), _controller[1].get())
    , _last_frame(clock::now())
    , _last_second(clock::now())
    , _frame_counter(0)
    , _rewinding(false)
{
}

NES::~NES()
{}

uint8_t NES::cpu_read(uint16_t addr) const
{
    return cpu.read(addr);
}

void NES::pull_NMI()
{
    cpu.pull_NMI();
}

void NES::pull_IRQ()
{
    cpu.pull_IRQ();
}

void NES::release_IRQ()
{
    cpu.release_IRQ();
}

void NES::on_frame()
{
    ++_frame_counter;

    _video->on_frame();

    for (auto &i : _input)
    {
        i->tick();
    }

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

    const auto target_frame_duration = std::chrono::seconds(1) / (60.0 * _rate);

    while ((clock::now() - _last_frame) < target_frame_duration)
        ;

    _last_frame = clock::now();
    
    if (_rewinding)
    {
        if (_cpu_states.size())
        {
            cpu = _cpu_states.back();
            ppu = _ppu_states.back();
            if (_cpu_states.size() > 1)
            {
                _cpu_states.pop_back();
                _ppu_states.pop_back();
            }
        }
    }
    else
    {
        if (_ppu_states.size() > 1024)
        {
            _cpu_states.erase(_cpu_states.begin());
            _ppu_states.erase(_ppu_states.begin());
        }
        _cpu_states.emplace_back(cpu);
        _ppu_states.emplace_back(ppu);
    }
}

void NES::rewind(bool on)
{
    _rewinding = on;
}

void NES::on_cpu_tick()
{
    apu.tick(_rate);
    ppu.tick3();
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
            cpu.update(_rate);
        }
    }
    catch (std::runtime_error const &e)
    {
        std::cout << "stop" << std::endl;
    }
}

uint8_t NES::read_chr(uint16_t addr) const
{
    return _rom->read_chr(addr);
}

uint8_t NES::read_nt(uint16_t addr) const
{
    return _rom->read_nt(addr);
}

void NES::write_chr(uint8_t value, uint16_t addr)
{
    _rom->write_chr(value, addr);
}

void NES::write_nt(uint8_t value, uint16_t addr)
{
    _rom->write_nt(value, addr);
}

void NES::put_pixel(uint8_t x, uint8_t y, PaletteIndex i)
{
    _video->put_pixel(x, y, i);
}
