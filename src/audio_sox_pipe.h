#ifndef AUDIO_SOX_PIPE_H
#define AUDIO_SOX_PIPE_H

#include "bus.h"

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
    size_t _index;
    std::array<int16_t, 2048> _samples;

    bool _done;
    std::atomic_flag _data_available;
    std::thread _thread;
};

#endif
