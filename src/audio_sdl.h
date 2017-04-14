#ifndef AUDIO_SDL_H
#define AUDIO_SDL_H

#include "bus.h"
#include "circular_buffer.h"

#include <iostream>
#include <array>
#include <thread>
#include <mutex>

class SDLAudioDevice : public IAudioDevice
{
public:
    static const size_t BUFFER_SIZE = 2048;

    SDLAudioDevice(IBus *);
    ~SDLAudioDevice();

    virtual void put_sample(int16_t);
    void on_buffer_request(float *buffer, size_t samples);

private:
    CircularBuffer<float, BUFFER_SIZE * 2> _in;
    IBus *_bus;
    int _device{0};
    bool _unpaused{false};
    double _rate{1.0};
};

#endif
