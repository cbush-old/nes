#ifndef IMAGE_H
#define IMAGE_H

#include <cstdint>

extern "C" {

void save_image(const char* path, uint32_t width, uint32_t height, uint32_t const* rgba_data);

}

#endif
