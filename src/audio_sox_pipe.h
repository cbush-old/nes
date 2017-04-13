#ifndef AUDIO_SOX_PIPE_H
#define AUDIO_SOX_PIPE_H

#include "bus.h"

#include <cstdio>
#include <thread>

template<typename T, size_t N>
class CircularBuffer
{
public:
    CircularBuffer()
        : _in(0)
        , _out(0)
        , _buffer{0}
    {}

    template<typename U>
    void write(U &&v)
    {
        _buffer[_in] = std::forward<U>(v);
        _in = (_in + 1) % _buffer.size();
    }

    size_t read(T *output, size_t count)
    {
        size_t n = 0;
        for (size_t i = 0; i < count && _out != _in; ++i)
        {
            output[i] = _buffer[_out];
            _out = (_out + 1) % _buffer.size();
            ++n;
        }
        return n;
        /*
        if (_out < _in)
        {
            n = std::min(count, _in - _out);
            std::memcpy(output, _buffer.data() + _out, n);
            _out = (_out + n) % _buffer.size();
        }
        else if (_in < _out)
        {
            size_t front = _in;
            size_t back = _buffer.size() - _out;
            
            if (count <= back)
            {
                n = count;
                std::memcpy(output, _buffer.data() + _out, n);
                _out = (_out + n) % _buffer.size();
            }
            else
            {
                std::memcpy(output, _buffer.data() + _out, back);
                _out = std::min(front, count);
                std::memcpy(output, _buffer.data(), _out);
                n = std::min(count, front + back);
            }
        }
        return n;
        */
    }

private:
    std::atomic<size_t> _in, _out;
    std::array<T, N> _buffer;
};


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
