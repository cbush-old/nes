#ifndef IO_H
#define IO_H

#include <iostream>
#include <thread>
#include <cmath>
#include <ctime>
#include <map>
#include <iomanip>
#include <functional>
#include <cctype>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_opengl.h>

class IO {
  
  friend class CPU;
  friend class PPU;
  
  private:
    SDL_Window *window;
    SDL_GLContext glcon;
    
  private:
    uint8_t handle_input();

    int button_index { 0 };
    uint8_t button_state[8];
    
    uint8_t input_state(uint8_t i);
    void strobe();
    void swap();
    void clear();
    void put_pixel(int x, int y, char r, char g, char b);
    
  public:
    IO();
    IO(IO const&) = delete;
    ~IO();

};

#endif
