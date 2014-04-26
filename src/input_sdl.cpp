#include "input_sdl.h"

#include <map>

#include <SDL2/SDL.h>

using Button = IController::Button;
using ButtonState = IController::ButtonState;

const std::map<int, Button> lookup {
  { SDLK_RIGHT, Button::CONTROL_RIGHT }, 
  { SDLK_LEFT, Button::CONTROL_LEFT }, 
  { SDLK_DOWN, Button::CONTROL_DOWN }, 
  { SDLK_UP, Button::CONTROL_UP }, 
  { SDLK_r, Button::CONTROL_START }, 
  { SDLK_s, Button::CONTROL_SELECT }, 
  { SDLK_d, Button::CONTROL_B }, 
  { SDLK_f, Button::CONTROL_A }, 
};

SDLInputDevice::SDLInputDevice(IController& controller)
  : port(controller)
  {}

void SDLInputDevice::tick() {

  SDL_Event e;

  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
      auto const& i = lookup.find(e.key.keysym.sym);
      if (i != lookup.end()) {
        port.set_button_state(i->second, (e.type == SDL_KEYDOWN) * IController::BUTTON_ON);
      }
    } else if (e.type == SDL_QUIT) {
      throw 1;
    }
  }

}
