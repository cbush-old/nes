#ifndef AUDIO_SOX_PIPE_H
#define AUDIO_SOX_PIPE_H

#include "bus.h"

#include <cstdio>

class SoxPipeAudioDevice : public IAudioDevice
{
public:
    SoxPipeAudioDevice();
    ~SoxPipeAudioDevice();

public:
    void put_sample(int16_t);

private:
    FILE *_pipe;
};

#endif
