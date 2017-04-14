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
    CircularBuffer()
        : _done(false)
        , _buffer{0}
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
    void blocking_read(size_t count, Fn process)
    {
        /*
        while (available() < count)
        {
            if (_done)
            {
                return;
            }
        }
        */

        while (count > 0)
        {
            if (_done)
            {
                return;
            }

            if (_reader == _writer)
            {
                continue;
            }

            lock guard(_mutex);

            size_t n = 0;
            size_t reader = _reader;
            size_t writer = _writer;
            if (reader < writer)
            {
                n = std::min(count, writer - reader);
                auto consumed = process(_buffer.data() + reader, n);
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
            advance_reader(n);
            assert(n <= count);
            count -= n;
        }
    }

    void kill()
    {
        _done = true;
    }

private:
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

    void advance_reader(size_t count)
    {
        size_t reader = _reader;
        _reader = (reader + count) % N;
    }
    
    void increment_writer()
    {
        _writer = (_writer + 1) % N;
        if (_writer == _reader)
        {
            lock guard(_mutex);
            if (_writer == _reader)
            {
                std::cout << "Writer overrun! writer == reader == " << _writer << '\n';
                advance_reader(1);
            }
        }
    }

    struct DummyMutex
    {
        void lock()
        {
        }

        void unlock()
        {
        }
    };
    
    using MutexType = std::mutex;

    using lock = std::lock_guard<MutexType>;
    MutexType _mutex;
    bool _done;
    std::array<T, N> _buffer;
    std::atomic<size_t> _reader;
    std::atomic<size_t> _writer;
};
