#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <mutex>
#include <condition_variable>

class semaphore
{
public:
    semaphore() {}
    semaphore(semaphore const &) = delete;
    semaphore &operator=(semaphore const &) = delete;
    ~semaphore() {}

public:
    inline void wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (!_count)
        {
            _condition.wait(lock);
        }
        --_count;
    }

    inline void signal()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        ++_count;
        _condition.notify_one();
    }

private:
    std::mutex _mutex;
    std::condition_variable _condition;
    unsigned _count{ 0 };
};

#endif
