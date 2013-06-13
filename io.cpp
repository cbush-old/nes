#include "io.h"
#include "bus.h"

using std::ifstream;
using std::cout;
using std::setw;
using std::hex;
using std::vector;
using std::thread;
using std::string;
using std::exception;
using std::runtime_error;

uint8_t IO::handle_input(){
  SDL_Event e;
  while(SDL_PollEvent(&e)){
    switch(e.type){
      case SDL_KEYDOWN:
        switch(e.key.keysym.sym){
          #define BUTTON_ON 0xff
          case SDLK_RIGHT: button_state[0] = BUTTON_ON; break;
          case SDLK_LEFT:  button_state[1] = BUTTON_ON; break;
          case SDLK_DOWN:  button_state[2] = BUTTON_ON; break;
          case SDLK_UP:    button_state[3] = BUTTON_ON; break;
          case SDLK_r:  button_state[4] = BUTTON_ON; break;
          case SDLK_s:  button_state[5] = BUTTON_ON; break;
          case SDLK_d:  button_state[6] = BUTTON_ON; break;
          case SDLK_f:  button_state[7] = BUTTON_ON; break;
          #undef BUTTON_ON
          case SDLK_1:  
            bus::save_state(); 
            e.type = 0;
            break;
          case SDLK_2:  
            bus::restore_state(); 
            e.type = 0;
            break;
        }
        return 0;
      case SDL_KEYUP:
        switch(e.key.keysym.sym){
          case SDLK_RIGHT: button_state[0] = 0x0; break;
          case SDLK_LEFT:  button_state[1] = 0x0; break;
          case SDLK_DOWN:  button_state[2] = 0x0; break;
          case SDLK_UP:    button_state[3] = 0x0; break;
          case SDLK_r:    button_state[4] = 0x0; break;
          case SDLK_s:   button_state[5] = 0x0; break;
          case SDLK_d:  button_state[6] = 0x0; break;
          case SDLK_f: button_state[7] = 0x0; break;
          
        }
        return 0;
      case SDL_QUIT:
        return 1;
    }
  }
}

uint8_t IO::input_state(uint8_t i){
  if(i == 2) return 0;
  if(button_index == -1)
    return 1;
  return button_state[button_index--];
}

void IO::strobe(){
  button_index = 7;
}

void IO::swap(){
  SDL_GL_SwapWindow(window);
  glClear(GL_COLOR_BUFFER_BIT);
}

void IO::clear(){
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(window);
}

void IO::put_pixel(int x, int y, char r, char g, char b){
  glColor3b(r,g,b);
  glBegin(GL_POINTS);
  glVertex2i(x,y);
  glEnd();
}

IO::IO():
  window(
    SDL_CreateWindow(
      "",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
      512,480,SDL_WINDOW_OPENGL
      //256,240,SDL_WINDOW_OPENGL
    )
  ),
  glcon(
    SDL_GL_CreateContext(window)
  )
  {
  glMatrixMode(GL_PROJECTION|GL_MODELVIEW);
  glLoadIdentity();
  glOrtho(0,256,240,0,0,1);
  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT);
  swap();
}

IO::~IO(){
  SDL_GL_DeleteContext(glcon);
  SDL_DestroyWindow(window);
}
