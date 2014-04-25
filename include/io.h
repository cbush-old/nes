#ifndef IO_H
#define IO_H

#include <cstdint>
#include <vector>

class IO {
  public:
    uint8_t input_state(uint8_t i);
    uint8_t handle_input();
    void strobe();
    void swap();
    void clear();
    void swap_with(std::vector<uint32_t> const&);

  public:
    IO();
    ~IO();

  private:
    struct SDL_Window *window;
    void *glcon;
    uint32_t texture;
    
  private:
    int button_index { 0 };
    uint8_t button_state[8];

};

#endif
