#include "audio_sox_pipe.h"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <unistd.h>

SoxPipeAudioDevice::SoxPipeAudioDevice()
    : _pipe(nullptr)
    , _done(false)
    , _data_available ATOMIC_FLAG_INIT
    , _thread(&SoxPipeAudioDevice::run, this)
{}

SoxPipeAudioDevice::~SoxPipeAudioDevice()
{
    _done = true;
    _thread.join();
}

void SoxPipeAudioDevice::run()
{
    _pipe = popen(
        "sox -q -t raw -c1 -b 16 -e signed-integer -r596591 - -t raw -c1 - rate 44800 "
        " | play -q -t raw -r44800 -c1 -b 16 -e signed-integer -",
        "w");
    
    if (!_pipe)
    {
        std::cerr << "Failed to open sox pipe.\n";
        return;
    }

    while (!_done)
    {
        static const size_t COUNT = 10;
        static int16_t sample[COUNT];
        auto amount = _buffer.read(sample, COUNT);
        if (amount > 0)
        {
            std::fwrite(sample, 2, amount, _pipe);
        }
    }

    pclose(_pipe);
}

void SoxPipeAudioDevice::put_sample(int16_t sample)
{
    static int i = 0;
    if (i++ % 3)
        return;
    _buffer.write(sample);
}
