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
    , cpu(this, _controller[0].get(), _controller[1].get())
    , _last_frame(clock::now())
    , _last_second(clock::now())
    , _frame_counter(0)
    , _frame_drop(0)
    , _last_frame_drop(0)
    , _rewinding(false)
{
    logi("sizeof CPU: %lu", sizeof(CPU));
    logi("sizeof PPU: %lu", sizeof(PPU));
    logi("sizeof ROM: %lu", sizeof(_rom));
    logi("sizeof total: %lu", sizeof(CPU) + sizeof(PPU) + sizeof(_rom));
}

NES::~NES()
{}

uint8_t NES::read_cpu(uint16_t addr)
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
        
        if (_last_frame_drop != _frame_drop)
        {
            std::printf("dropped %lu frames\n", _frame_drop);
            _last_frame_drop = _frame_drop;
        }
        _frame_counter = 0;
        _frame_drop = 0;
    }

#if REWIND
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
                _rom = std::move(_rom_states.back());
                _rom_states.pop_back();
            }
            else
            {
                _rom = _rom_states.back();
            }
        }
    }
    else
    {
        if (_ppu_states.size() > 1024)
        {
            _cpu_states.erase(_cpu_states.begin());
            _ppu_states.erase(_ppu_states.begin());
            _rom_states.erase(_rom_states.begin());
        }
        _cpu_states.emplace_back(cpu);
        _ppu_states.emplace_back(ppu);
        _rom_states.push_back(_rom);
    }
#endif

    const auto target_frame_duration = std::chrono::seconds(1) / (60.0 * _rate);

    if ((clock::now() - _last_frame) < target_frame_duration)
    {
        _video->on_frame();
    }
    else
    {
        ++_frame_drop;
    }

    _last_frame = clock::now();
}

void NES::rewind(bool on)
{
    _rewinding = on;
}

void NES::on_cpu_tick()
{
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
            apu.tick(_rate);
            ppu.tick3();
        }
    }
    catch (std::runtime_error const &e)
    {
        std::cout << "stop" << std::endl;
    }
}

uint8_t NES::read_chr(uint16_t addr)
{
    return _rom->read_chr(addr);
}

uint8_t NES::read_nt(uint16_t addr)
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

uint8_t NES::read_apu()
{
    return apu.read();
}

uint8_t NES::read_prg(uint16_t addr)
{
    return _rom->read_prg(addr);
}

uint8_t NES::read_ppu(uint16_t addr)
{
    switch (addr & 7)
    {
        case 2: return ppu.regr_status();
        case 4: return ppu.regr_OAM_data();
        case 7: return ppu.regr_data();
        default: // bad read
            return 0;
    }
}

void NES::write_apu(uint8_t value, uint16_t addr)
{
    apu.write(value, addr);
}

void NES::write_ppu(uint8_t value, uint16_t addr)
{
    switch (addr & 7)
    {
        case 0: ppu.regw_control(value); break;
        case 1: ppu.regw_mask(value); break;
        case 3: ppu.regw_OAM_address(value); break;
        case 4: ppu.regw_OAM_data(value); break;
        case 5: ppu.regw_scroll(value); break;
        case 6: ppu.regw_address(value); break;
        case 7: ppu.regw_data(value); break;
    }
}

void NES::write_prg(uint8_t value, uint16_t addr)
{
    _rom->write_prg(value, addr);
}

