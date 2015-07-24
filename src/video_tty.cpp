#include "video_tty.h"
#include <cstdio>

Framebuffer const* buf { nullptr };

char get_char(uint32_t x) {
  return x ? 'A' + x : ' ';
}

void TTYVideoDevice::set_buffer(Framebuffer const& buffer) {
  buf = &buffer;
}

void TTYVideoDevice::on_frame() {
  if (!buf) {
    return;
  }
  std::printf("\033[0;0H");
  for (size_t y = 0; y < 240; y += 5) {
    for (size_t x = 0; x < 256; x += 2) {
      std::printf("%c", get_char(buf->at(y * 256 + x)));
    }
    std::printf("\n");
  }
}
