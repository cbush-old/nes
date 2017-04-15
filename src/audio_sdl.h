#ifndef AUDIO_SDL_H
#define AUDIO_SDL_H

#include "bus.h"
#include "circular_buffer.h"

#include <array>
#include <iostream>
#include <mutex>
#include <thread>

class SDLAudioDevice : public IAudioDevice
{
public:
    static const size_t BUFFER_SIZE = 256;

    SDLAudioDevice(IBus *);

    ~SDLAudioDevice();

    virtual void put_sample(int16_t sample, size_t hz) override;

    void on_buffer_request(float *buffer, size_t samples);

private:
    CircularBuffer<int16_t, BUFFER_SIZE * 2> _in;
    IBus *_bus;
    int _device;
    bool _unpaused;
};

#endif
