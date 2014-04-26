#include <iostream>
#include <thread>
#include <cmath>
#include <ctime>
#include <map>
#include <iomanip>
#include <functional>
#include <cctype>
#include <fstream>

#include <string>
#include <stdexcept>
#include <sstream>

#include "io.h"
#include "bus.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <gl.h>
#include <glext.h>

#define AUDIO_BUFFER_SIZE 1000

#include <samplerate.h>

int a = 0;


void audio_callback(void*, uint8_t *stream, int length){
  for (int i = 0; i < length; ++i)
    stream[i] = 0; // fixme

  // Receive 39.9502901786x more samples than wanted
  // In:      1789773
  // Out:       44800

}

using std::ifstream;
using std::cout;
using std::setw;
using std::hex;
using std::vector;
using std::thread;
using std::string;
using std::exception;
using std::runtime_error;

uint8_t spool_i = 0;
size_t spool_l = 0;
size_t frame = 0;
bool unwinding = false;
std::vector<State> spool (256);

/*
uint8_t IO::handle_input() {
  #define SPOOL
  #ifdef SPOOL
  if(unwinding){
    if(spool_l > 0){
      bus::restore_state(spool[--spool_i]);
      spool_l--;
    } else {
      bus::restore_state(spool[spool_i]);
    }
  } else {//if(frame++){
    bus::get_state(spool[spool_i++]);
    if(spool_l < 255)
      spool_l++;
    
  }
  #endif
  
  SDL_Event e;

  while(SDL_PollEvent(&e)){
    switch(e.type){
      case SDL_KEYDOWN:
        switch(e.key.keysym.sym){
          case SDLK_RIGHT: button_state[CONTROL_RIGHT] = BUTTON_ON; break;
          case SDLK_LEFT:  button_state[CONTROL_LEFT] = BUTTON_ON; break;
          case SDLK_DOWN:  button_state[CONTROL_DOWN] = BUTTON_ON; break;
          case SDLK_UP:    button_state[CONTROL_UP] = BUTTON_ON; break;
          case SDLK_r:  button_state[CONTROL_START] = BUTTON_ON; break;
          case SDLK_s:  button_state[CONTROL_SELECT] = BUTTON_ON; break;
          case SDLK_d:  button_state[CONTROL_B] = BUTTON_ON; break;
          case SDLK_f:  button_state[CONTROL_A] = BUTTON_ON; break;
          case SDLK_1:  
            bus::save_state(); 
            e.type = 0;
            break;
          case SDLK_2:  
            bus::restore_state(); 
            e.type = 0;
            break;
          case SDLK_q:
            unwinding = true;
            break;
        }
        return 0;
      case SDL_KEYUP:
        switch(e.key.keysym.sym){
          case SDLK_RIGHT: button_state[CONTROL_RIGHT] = BUTTON_OFF; break;
          case SDLK_LEFT:  button_state[CONTROL_LEFT] = BUTTON_OFF; break;
          case SDLK_DOWN:  button_state[CONTROL_DOWN] = BUTTON_OFF; break;
          case SDLK_UP:    button_state[CONTROL_UP] = BUTTON_OFF; break;
          case SDLK_r:    button_state[CONTROL_START] = BUTTON_OFF; break;
          case SDLK_s:   button_state[CONTROL_SELECT] = BUTTON_OFF; break;
          case SDLK_d:  button_state[CONTROL_B] = BUTTON_OFF; break;
          case SDLK_f: button_state[CONTROL_A] = BUTTON_OFF; break;
          case SDLK_q: 
            unwinding = false; 
            break;
          
        }
        return 0;
      case SDL_QUIT:
        return 1;
    }
  }
  return 0;
}
*/

IController::ButtonState IO::read() {
  if(button_index == -1) {
    return 1;
  }
  return button_state[button_index--];
}

void IO::strobe() {
  button_index = 7;
}

void IO::swap(){
  SDL_GL_SwapWindow(window);
  glClear(GL_COLOR_BUFFER_BIT);
}

void IO::swap_with(std::vector<uint32_t> const& buffer){
  glTexSubImage2D(
    GL_TEXTURE_2D,
    0,
    0,
    0,
    256,
    240,
    GL_RGBA, 
    GL_UNSIGNED_INT_8_8_8_8, 
    (const GLvoid*)buffer.data()
  );
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(0.0, 0.0f);  glVertex2i(0,0);
  glTexCoord2f(1.0, 0.0f);  glVertex2i(256,0);
  glTexCoord2f(0.0, 1.0f);  glVertex2i(0,240);
  glTexCoord2f(1.0, 1.0f);  glVertex2i(256,240);
  glEnd();
  SDL_GL_SwapWindow(window);
  glClear(GL_COLOR_BUFFER_BIT);
}

void IO::clear(){
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(window);
}


IO::IO():
  window(
    SDL_CreateWindow(
      "",
      SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
      512,480,
      SDL_WINDOW_OPENGL
    )
  ),
  glcon(
    new SDL_GLContext(SDL_GL_CreateContext(window))
  )
{

  glMatrixMode(GL_PROJECTION|GL_MODELVIEW);
  glLoadIdentity();
  glOrtho(0,256,240,0,0,1);
  glClearColor(0,0.6,0,1);
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(
    GL_TEXTURE_2D, 
    0, 
    GL_RGBA, 
    256,
    240,
    0,
    GL_RGBA,
    GL_UNSIGNED_INT_8_8_8_8, 
    nullptr
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  swap();
  
  
  
  SDL_AudioSpec obtained, desired;
  
  desired.freq = 44800;
  desired.format = AUDIO_S16LSB;
  desired.channels = 1;
  desired.samples = AUDIO_BUFFER_SIZE * 2;
  desired.callback = audio_callback;
  desired.userdata = nullptr;
  
  int audio = SDL_OpenAudio(&desired, &obtained);

  std::cout << audio << '\n';
  std::cout << obtained.freq << '\n';
  
  SDL_PauseAudio(0);
  
}

IO::~IO() {
  SDL_CloseAudio();
  
  glDeleteTextures(1, &texture);
  SDL_GL_DeleteContext(*static_cast<SDL_GLContext*>(glcon));
  delete static_cast<SDL_GLContext*>(glcon);
  SDL_DestroyWindow(window);
}


IController::ButtonState GamePad::read() {
  if(button_index == -1) {
    return 1;
  }
  return button_state[button_index--];
}

void GamePad::strobe() {
  button_index = 7;
}

void GamePad::set_button_state(IController::Button button, IController::ButtonState state) {
  button_state[button] = state;
}

SDLVideoDevice::SDLVideoDevice():
  window(
    SDL_CreateWindow(
      "",
      SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
      512,480,
      SDL_WINDOW_OPENGL
    )
  ),
  glcon(
    new SDL_GLContext(SDL_GL_CreateContext(window))
  )
{

  glMatrixMode(GL_PROJECTION|GL_MODELVIEW);
  glLoadIdentity();
  glOrtho(0,256,240,0,0,1);
  glClearColor(0,0.6,0,1);
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(
    GL_TEXTURE_2D, 
    0, 
    GL_RGBA, 
    256,
    240,
    0,
    GL_RGBA,
    GL_UNSIGNED_INT_8_8_8_8, 
    nullptr
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}

SDLVideoDevice::~SDLVideoDevice() {
  glDeleteTextures(1, &texture);
  SDL_GL_DeleteContext(*static_cast<SDL_GLContext*>(glcon));
  delete static_cast<SDL_GLContext*>(glcon);
  SDL_DestroyWindow(window);
}


void SDLVideoDevice::set_buffer(Raster const& raster) {
  glTexSubImage2D(
    GL_TEXTURE_2D,
    0,
    0,
    0,
    256,
    240,
    GL_RGBA, 
    GL_UNSIGNED_INT_8_8_8_8, 
    (const GLvoid*)raster.data()
  );
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(0.0, 0.0f);  glVertex2i(0,0);
  glTexCoord2f(1.0, 0.0f);  glVertex2i(256,0);
  glTexCoord2f(0.0, 1.0f);  glVertex2i(0,240);
  glTexCoord2f(1.0, 1.0f);  glVertex2i(256,240);
  glEnd();
  SDL_GL_SwapWindow(window);
  glClear(GL_COLOR_BUFFER_BIT);
}

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

