#include "audio_sdl.h"
#include "SDL.h"

#include <samplerate.h>

#include <iostream>
#include <thread>

const int AUDIO_BUFFER_SIZE = 2500;
const size_t FLUSH_SIZE = AUDIO_BUFFER_SIZE * 4;
const double RATIO = 44800.0 / 1789773.0;

using lock_guard = std::lock_guard<std::recursive_mutex>;


void audio_callback(void *userdata, uint8_t *stream, int length) {

  SDLAudioDevice& device = *(SDLAudioDevice*)userdata;
  device.on_buffer_request(stream, length);

}

SDLAudioDevice::SDLAudioDevice() {

  _state = src_new(SRC_LINEAR, 1, &_error);

  SDL_InitSubSystem(SDL_INIT_AUDIO);

  SDL_AudioSpec obtained, desired;
  SDL_zero(desired);

  desired.freq = 44800;
  desired.format = AUDIO_S16LSB;
  desired.channels = 1;
  desired.samples = AUDIO_BUFFER_SIZE * 2;
  desired.callback = audio_callback;
  desired.userdata = this;

  _device = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0); // FIXME: handle different configurations
  if (_device == 0) {
    std::cerr << "Couldn't open audio device: " << SDL_GetError() << "\n";
    throw 0;
  } else {
    SDL_PauseAudioDevice(_device, 0);
    std::cout << obtained.freq << '\n';
  }

}


SDLAudioDevice::~SDLAudioDevice() {
  lock_guard lock(_mutex);
  _state = src_delete(_state);
  SDL_CloseAudioDevice(_device);
  SDL_QuitSubSystem(SDL_INIT_AUDIO);

  std::cout
    << "put_sample (inner): " << (_debug_put_sample_inner_time / (double)CLOCKS_PER_SEC) << "\n"
    << "put_sample (outer): " << (_debug_put_sample_outer_time / (double)CLOCKS_PER_SEC) << "\n"
    << "buffer fill (inner): " << (_debug_buffer_fill_inner_time / (double)CLOCKS_PER_SEC) << "\n"
    << "buffer fill (outer): " << (_debug_buffer_fill_outer_time / (double)CLOCKS_PER_SEC) << "\n";
}

void SDLAudioDevice::put_sample(int16_t sample) {

  lock_guard lock (_mutex);

  _in.push_back(sample / 32768.f);

  if (_in.size() > FLUSH_SIZE) {

    std::thread t0 { [this] {

      lock_guard lock (_mutex);

      SRC_DATA data;
      data.data_in = _in.data();
      data.data_out = _out.data() + _out.size();
      data.input_frames = _in.size();
      data.output_frames = _out.available_size();
      data.src_ratio = 44800.0 / 1789773.0;
      data.end_of_input = 0;
      int error = src_process(_state, &data);

      if (error) {
        std::cerr << "SRC error: " << src_strerror(error) << "\n";
      }

      _in.flush(data.input_frames_used);
      _out.add(data.output_frames_gen);

    }};
    t0.detach();

  }

}


void SDLAudioDevice::on_buffer_request(uint8_t *stream, int length) {

  lock_guard lock(_mutex);

  length /= 2;

  int available = _out.size();

  if (available < length) {
    // underrun
    length = available;
  }

  src_float_to_short_array((const float*)_out.data(), (short*)stream, length);

  std::thread t0 { [this, length] {
    lock_guard lock(_mutex);
    _out.flush(length);
  } };

  t0.detach();
}
