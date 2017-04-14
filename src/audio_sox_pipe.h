#ifndef AUDIO_SOX_PIPE_H
#define AUDIO_SOX_PIPE_H

#include "bus.h"
#include "circular_buffer.h"

#include <cstdio>
#include <thread>

class SoxPipeAudioDevice : public IAudioDevice
{
public:
    SoxPipeAudioDevice();
    ~SoxPipeAudioDevice();

    virtual void put_sample(int16_t) override;

private:
    void run();

    FILE *_pipe;

    CircularBuffer<int16_t, 4096> _buffer;

    std::atomic<bool> _done;
    std::atomic_flag _data_available;
    std::thread _thread;
};

#endif
