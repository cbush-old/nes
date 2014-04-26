#ifndef IO_H
#define IO_H

#include <cstdint>
#include <vector>
#include "bus.h"

class IO : IController {
  public:
    IO();
    ~IO();

  public:
    ButtonState read();
    uint8_t handle_input();
    void strobe();
    void swap();
    void clear();
    void swap_with(std::vector<uint32_t> const&);

  private:
    struct SDL_Window *window;
    void *glcon;
    uint32_t texture;
    
  private:
    int button_index { 0 };
    ButtonState button_state[8];

};

class GamePad : public IController {
  public:
    ~GamePad(){}

  public:
    ButtonState read();
    void strobe();
    void set_button_state(Button button, ButtonState state);

  private:
    int button_index { 0 };
    ButtonState button_state[8];

};

class SDLVideoDevice : public IVideoDevice {
  public:
    SDLVideoDevice();
    ~SDLVideoDevice();

  public:
    void set_buffer(Raster const& raster);

  private:
    struct SDL_Window *window;
    void *glcon;
    uint32_t texture;

};

class SDLAudioDevice : public IAudioDevice {
  public:
    ~SDLAudioDevice(){}

};

class SDLInputDevice : public IInputDevice {
  public:
    SDLInputDevice(IController& controller);
    ~SDLInputDevice(){}

  public:
    void tick();

  private:
    IController& port;
};


#endif
