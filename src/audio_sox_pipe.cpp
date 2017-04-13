#include "audio_sox_pipe.h"

#include <unistd.h>

SoxPipeAudioDevice::SoxPipeAudioDevice()
{
    _pipe = popen(
        "sox -t raw -c1 -b 16 -e signed-integer -r1789773 - -t raw -c2 - rate 48000 "
        " | aplay -fdat",
        "w");
}

SoxPipeAudioDevice::~SoxPipeAudioDevice()
{
    pclose(_pipe);
}

void SoxPipeAudioDevice::put_sample(int16_t sample)
{
    fputc(sample, _pipe);
    fputc(sample >> 8, _pipe);
}
