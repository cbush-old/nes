#include "audio_sdl.h"
#include "SDL.h"

#include <samplerate.h>

#include <iostream>
#include <thread>
#include <cstdio>

static const double OUTPUT_FREQUENCY = 44800.0;
const int AUDIO_BUFFER_SIZE = SDLAudioDevice::BUFFER_SIZE;

SDLAudioDevice::SDLAudioDevice(IBus *bus)
    : _bus(bus)
{
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    SDL_AudioSpec obtained, desired;
    SDL_zero(desired);

    desired.freq = static_cast<int>(OUTPUT_FREQUENCY);
    desired.format = AUDIO_F32;
    desired.channels = 1;
    desired.samples = AUDIO_BUFFER_SIZE;
    desired.callback = nullptr;
    desired.userdata = this;

    _device = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0); // FIXME: handle different configurations
    if (_device == 0)
    {
        std::cerr << "Couldn't open audio device: " << SDL_GetError() << "\n";
        throw 0;
    }
    else
    {
        std::cout << obtained.freq << '\n';
        std::cout << "Unpausing audio...\n";
        SDL_PauseAudioDevice(_device, 0);
        _unpaused = true;
    }
}

SDLAudioDevice::~SDLAudioDevice()
{
    SDL_CloseAudioDevice(_device);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void SDLAudioDevice::put_sample(int16_t sample)
{
    static double f = 0;
    static int x = 0;
    ++x;
    
    const auto ratio = (1789773.0 / OUTPUT_FREQUENCY);
    if (x >= ratio)
    {
        f /= ratio;
        _in.emplace(f);
        f = 0;
        x = 0;
        if (_in.available() > BUFFER_SIZE)
        {
            _in.read(BUFFER_SIZE, [this](float *stream, size_t count)
            {
                SDL_QueueAudio(_device, stream, count * sizeof(float));
                return count;
            });
        }
    }

    f += sample / (float)0x7fff;
}
