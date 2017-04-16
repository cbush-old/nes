#include "video_sdl.h"

#include "image.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <gl.h>
#include <glext.h>

#include <cassert>
#include <cmath>
#include <ctime>
#include <iostream>
#include <sstream>

// Lookup table for palette index to 32-bit RGB
static const uint32_t RGB[64]
{
    0x7C7C7CFF, 0x0000FCFF, 0x0000BCFF, 0x4428BCFF, 0x940084FF, 0xA80020FF, 0xA81000FF, 0x881400FF,
    0x503000FF, 0x007800FF, 0x006800FF, 0x005800FF, 0x004058FF, 0x000000FF, 0x000000FF, 0x000000FF,
    0xBCBCBCFF, 0x0078F8FF, 0x0058F8FF, 0x6844FCFF, 0xD800CCFF, 0xE40058FF, 0xF83800FF, 0xE45C10FF,
    0xAC7C00FF, 0x00B800FF, 0x00A800FF, 0x00A844FF, 0x008888FF, 0x000000FF, 0x000000FF, 0x000000FF,
    0xF8F8F8FF, 0x3CBCFCFF, 0x6888FCFF, 0x9878F8FF, 0xF878F8FF, 0xF85898FF, 0xF87858FF, 0xFCA044FF,
    0xF8B800FF, 0xB8F818FF, 0x58D854FF, 0x58F898FF, 0x00E8D8FF, 0x787878FF, 0x000000FF, 0x000000FF,
    0xFCFCFCFF, 0xA4E4FCFF, 0xB8B8F8FF, 0xD8B8F8FF, 0xF8B8F8FF, 0xF8A4C0FF, 0xF0D0B0FF, 0xFCE0A8FF,
    0xF8D878FF, 0xD8F878FF, 0xB8F8B8FF, 0xB8F8D8FF, 0x00FCFCFF, 0xF8D8F8FF, 0x000000FF, 0x000000FF,
};

uint32_t palette_index_to_rgba(PaletteIndex i)
{
    return RGB[i];
}

SDLVideoDevice::SDLVideoDevice()
    : _window{SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 480, SDL_WINDOW_OPENGL)}
    , _gl_context(SDL_GL_CreateContext(_window))
    , _texture(0)
    , _buffer_dirty(false)
{
    SDL_GL_SetSwapInterval(1);

    glMatrixMode(GL_PROJECTION | GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(0, 256, 240, 0, 0, 1);
    glClearColor(0, 0.6, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        256,
        240,
        0,
        GL_RGBA,
        GL_UNSIGNED_INT_8_8_8_8,
        _buffer.data()
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

SDLVideoDevice::~SDLVideoDevice()
{
    glDeleteTextures(1, &_texture);
    SDL_GL_DeleteContext(_gl_context);
    SDL_DestroyWindow(_window);
}

void SDLVideoDevice::put_pixel(uint8_t x, uint8_t y, PaletteIndex i)
{
    auto &pixel_in_buffer = _buffer[y * 256 + (x & 0xff)];
    auto p = RGB[i];
    _buffer_dirty |= (pixel_in_buffer != p);
    pixel_in_buffer = p;
}

void SDLVideoDevice::on_frame()
{
    if (_buffer_dirty)
    {
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0, // level
            0, // xoffset
            0, // yoffset
            256, // width
            240, // height
            GL_RGBA,
            GL_UNSIGNED_INT_8_8_8_8,
            static_cast<const GLvoid *>(_buffer.data())
        );
        _buffer_dirty = false;
    }
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0, 0.0f); glVertex2i(0, 0);
    glTexCoord2f(1.0, 0.0f); glVertex2i(256, 0);
    glTexCoord2f(0.0, 1.0f); glVertex2i(0, 240);
    glTexCoord2f(1.0, 1.0f); glVertex2i(256, 240);
    glEnd();

    SDL_GL_SwapWindow(_window);
    glClear(GL_COLOR_BUFFER_BIT);
}

void screenshot()
{
    std::stringstream ss;
    ss << "nes-snap-" << time(NULL) << ".png";
    logi("create %s", ss.str().c_str());
    //save_image(ss.str().c_str(), 256, 240, _buffer.data());
}
