#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <iostream>
#include <mutex>
#include <thread>

template<typename T, size_t N>
class CircularBuffer
{
public:
    static_assert((N & ~(N - 1)) == N, "N must be a power of 2!");

    CircularBuffer()
        : _buffer{0}
        , _reader(0)
        , _writer(0)
    {}
    
    ~CircularBuffer()
    {}

    template<typename... Tp>
    void emplace(Tp &&... args)
    {
        _buffer[_writer] = T(std::forward<Tp>(args)...);
        increment_writer();
    }

    template<typename Fn>
    size_t read(size_t count, Fn process)
    {
        size_t reader = _reader;
        size_t writer = _writer;
        
        if (reader == writer)
        {
            return 0;
        }

        size_t n = 0;
        if (reader < writer)
        {
            n = std::min(count, writer - reader);
            auto consumed = process(_buffer.data() + reader, n);
            (void)consumed;
            assert(consumed == n);
        }
        else if (writer < reader)
        {
            const auto first = std::min(count, N - reader);
            size_t second = 0;
            auto consumed = process(_buffer.data() + reader, first);
            assert(consumed == first);

            if (first < count)
            {
                second = std::min<size_t>(count - first, writer);
                consumed = process(_buffer.data(), second);
                assert(consumed == second);
            }
            n = first + second;
        }
        //std::printf("reader=%lu, writer=%lu, n=%lu\n", reader, writer, n);
        advance_reader(n);
        assert(n <= count);
        return n;
    }

    size_t available() const
    {
        size_t reader = _reader;
        size_t writer = _writer;
        if (reader == writer)
        {
            return 0;
        }
        if (reader < writer)
        {
            return writer - reader;
        }
        return (N - reader) + writer;
    }

private:
    void advance_reader(size_t count)
    {
        _reader = (_reader + count) & (N - 1);
    }

    void increment_writer()
    {
        _writer = (_writer + 1) & (N - 1);
        if (_writer == _reader)
        {
            advance_reader(1);
        }
    }

    std::array<T, N> _buffer;
    size_t _reader;
    size_t _writer;
};
