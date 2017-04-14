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
        while (count > 0)
        {
            if (_done)
            {
                return;
            }

            if (_reader == _writer)
            {
                std::this_thread::yield();
                continue;
            }

            if (!_mutex.try_lock())
            {
                continue;
            }

            lock guard(_mutex, std::adopt_lock);

            size_t n = 0;
            if (_reader < _writer)
            {
                n = std::min(count, _writer - _reader);
                auto consumed = process(_buffer.data() + _reader, n);
                assert(consumed == n);
            }
            else if (_writer < _reader)
            {
                const auto first = std::min(count, N - _reader);
                size_t second = 0;
                auto consumed = process(_buffer.data() + _reader, first);
                assert(consumed == first);

                if (first < count)
                {
                    second = std::min<size_t>(count - first, _writer);
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
    void advance_reader(size_t count)
    {
        _reader = (_reader + count) % N;
    }
    
    void increment_writer()
    {
        lock guard(_mutex);
        _writer = (_writer + 1) % N;
        if (_writer == _reader)
        {
            std::cout << "Writer overrun!\n";
            advance_reader(N / 2);
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
        bool try_lock()
        {
            return true;
        }
    };
    
    using MutexType = DummyMutex;

    using lock = std::lock_guard<MutexType>;
    MutexType _mutex;
    bool _done;
    std::array<T, N> _buffer;
    std::atomic<size_t> _reader;
    std::atomic<size_t> _writer;
};
