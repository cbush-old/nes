#ifndef AUDIO_SDL_H
#define AUDIO_SDL_H

#include "bus.h"

#include <iostream>
#include <array>
#include <thread>
#include <mutex>

template<typename T, size_t SIZE>
class Buffer {
  private:
    T _data[SIZE];
    size_t _cursor;

  public:
    void push_back(T v) {
      if (_cursor < SIZE - 1) {
        _data[_cursor++] = v;
      }
    }

    T* data() {
      return _data;
    }

    size_t size() const {
      return _cursor;
    }

    size_t available_size() const {
      return SIZE - _cursor;
    }

    /**
     * @brief remove elements from the beginning of the buffer
     * @param count the number of elements to remove
     **/
    void flush(size_t count) {
      if (count >= _cursor) {
        _cursor = 0;
        return;
      }

      size_t remaining = _cursor - count;
      for (size_t i = 0; i < remaining; ++i) {
        _data[i] = _data[count + i];
      }
      _cursor -= count - 1;
    }

    /**
     * @brief update the buffer cursor with the number of elements that were 
     *        added directly to the data element.
     * @param count the number of elements that were added
     **/
    void add(size_t count) {
      _cursor += count;
      if (_cursor >= SIZE) {
        _cursor = SIZE;
      }
    }

};

class SDLAudioDevice : public IAudioDevice {
  public:
    SDLAudioDevice();
    ~SDLAudioDevice();

  public:
    virtual void put_sample(int16_t);
    void on_buffer_request(uint8_t*, int);

  private:
    // Secret Rabbit Code
    struct SRC_STATE_tag *_state { NULL };
    int _error;
    Buffer<float, 1000000> _in, _out;

  private:
    std::recursive_mutex _mutex, _in_mutex;
    int _device { 0 };

};

#endif
