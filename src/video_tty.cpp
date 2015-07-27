#include "video_tty.h"
#include <cstdio>

Framebuffer buffer;

char get_char(uint32_t x) {
  return x ? 'A' + x : ' ';
}

void TTYVideoDevice::put_pixel(uint8_t x, uint8_t y, PaletteIndex i) {
  buffer[y * 256 + (x % 256)] = i;
}

void TTYVideoDevice::on_frame() {
  std::printf("\033[0;0H");
  for (size_t y = 0; y < 240; y += 5) {
    for (size_t x = 0; x < 256; x += 2) {
      std::printf("%c", get_char(buffer.at(y * 256 + x)));
    }
    std::printf("\n");
  }
}
