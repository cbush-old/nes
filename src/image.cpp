#include "image.h"
#include "log.h"
#include <Imlib2.h>

extern "C" {

void save_image(const char* path, uint32_t w, uint32_t h, uint32_t const* rgba_data) {
  logi("saving image %s", path);
  Imlib_Image image (imlib_create_image(w, h));
  imlib_context_set_image(image);

  for (size_t y = 0; y < h; ++y) {
    for (size_t x = 0; x < w; ++x) {
      uint32_t rgba = rgba_data[y * w + x];
      uint8_t r = (rgba >> 24),
        g = (rgba >> 16) & 0xff,
        b = (rgba >> 8) & 0xff,
        a = rgba & 0xff;
      imlib_context_set_color(r, g, b, a);
      imlib_image_fill_rectangle(x, y, 1, 1);
    }
  }

  imlib_save_image(path);
  imlib_free_image();

}

}
