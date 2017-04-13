#include "audio_sox_pipe.h"

#include <cstdio>
#include <iostream>
#include <unistd.h>

SoxPipeAudioDevice::SoxPipeAudioDevice()
    : _pipe(nullptr)
    , _index(0)
    , _done(false)
    , _data_available ATOMIC_FLAG_INIT
    //, _thread(&SoxPipeAudioDevice::run, this)
{
    (void)_pipe;
}

SoxPipeAudioDevice::~SoxPipeAudioDevice()
{
    _done = true;
    _thread.join();
}

void SoxPipeAudioDevice::run()
{
#if 0
    _pipe = popen(
        "sox -q -t raw -c1 -b 16 -e signed-integer -r1789773 - -t raw -c2 - rate 44100 "
        " | play -q -t raw -r44100 -c1 -b 16 -e signed-integer -",
        "w");
    
    if (!_pipe)
    {
        std::cerr << "Failed to open sox pipe.\n";
        return;
    }
    
    while (!_done)
    {
    /*
        if (!_data_available.test_and_set(std::memory_order_acquire))
        {
            std::fwrite(_samples.data(), 2, _samples.size(), _pipe);
            _index = 0;
        }
    */
    }
    
    pclose(_pipe);
#endif
}

void SoxPipeAudioDevice::put_sample(int16_t sample)
{
    _samples[_index] = sample;
    ++_index;
    if (_index == _samples.size())
    {
        //_data_available.clear(std::memory_order_release);
    }
}
