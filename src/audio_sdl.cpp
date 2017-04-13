#include "audio_sdl.h"
#include "SDL.h"

#include <samplerate.h>

#include <iostream>
#include <thread>
#include <cstdio>

const int AUDIO_BUFFER_SIZE = 2500;

void audio_callback(void *userdata, uint8_t *stream, int length)
{
    std::memset(stream, 0, length);
    SDLAudioDevice &device = *(SDLAudioDevice *)userdata;
    device.on_buffer_request(stream, length);
}

SDLAudioDevice::SDLAudioDevice(IBus *bus)
    : _state(src_new(SRC_LINEAR, 1, &_error))
    , _bus(bus)
{

    memset(_in.data(), 0, BUFFER_SIZE * sizeof(float));
    memset(_out.data(), 0, BUFFER_SIZE * sizeof(float));

    SDL_InitSubSystem(SDL_INIT_AUDIO);

    SDL_AudioSpec obtained, desired;
    SDL_zero(desired);

    desired.freq = 1789773 / 3;
    desired.format = AUDIO_S16LSB;
    desired.channels = 1;
    desired.samples = AUDIO_BUFFER_SIZE * 2;
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
        SDL_PauseAudioDevice(_device, 0);
    }
}

SDLAudioDevice::~SDLAudioDevice()
{
    _state = src_delete(_state);
    SDL_CloseAudioDevice(_device);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void SDLAudioDevice::put_sample(int16_t sample)
{
    static int x = 0;
    if (x % 3)
    {
        return;
    }
    
    x = 0;
    SDL_QueueAudio(_device, &sample, 2);
/*
    _in.push_back(sample / 32768.f);

    if (_in.size() > FLUSH_SIZE && _out.available_size() && !_dumping)
    {
        _dumping = true;

        SRC_DATA data;
        data.data_in = _in.data();
        data.data_out = _out.data() + _out.size();
        data.input_frames = _in.size();
        data.output_frames = _out.available_size();
        data.src_ratio = RATIO;
        data.end_of_input = 0;
        int error = src_process(_state, &data);

        if (error)
        {
            std::cerr << "SRC error: " << src_strerror(error) << "\n";
        }

        _in.flush(data.input_frames_used);
        _out.add(data.output_frames_gen);

        bool ready = _out.size() > AUDIO_BUFFER_SIZE * 2;
        if (ready && !_unpaused)
        {
            std::cout << "Unpausing audio...\n";
            SDL_PauseAudioDevice(_device, 0);
            _unpaused = true;
        }

        _dumping = false;

    }
*/
}

void SDLAudioDevice::on_buffer_request(uint8_t *stream, int length)
{
    length /= 2;

    int available = _out.size();

    if (available < length)
    {
        // underrun
        std::cerr << "Underrun!\n";
        length = available;
    }

    src_float_to_short_array((const float *)_out.data(), (short *)stream, length);

    _out.flush(length);
}
