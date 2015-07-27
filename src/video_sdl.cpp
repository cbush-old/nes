#include "video_sdl.h"
#include "SDL.h"

#include <iostream>
#include <cmath>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <gl.h>
#include <glext.h>


// Lookup table for palette index to 32-bit RGB
static const uint32_t RGB[64] {
  0x7C7C7CFF, 0x0000FCFF, 0x0000BCFF, 0x4428BCFF, 0x940084FF, 0xA80020FF, 0xA81000FF, 0x881400FF,
  0x503000FF, 0x007800FF, 0x006800FF, 0x005800FF, 0x004058FF, 0x000000FF, 0x000000FF, 0x000000FF,
  0xBCBCBCFF, 0x0078F8FF, 0x0058F8FF, 0x6844FCFF, 0xD800CCFF, 0xE40058FF, 0xF83800FF, 0xE45C10FF,
  0xAC7C00FF, 0x00B800FF, 0x00A800FF, 0x00A844FF, 0x008888FF, 0x000000FF, 0x000000FF, 0x000000FF,
  0xF8F8F8FF, 0x3CBCFCFF, 0x6888FCFF, 0x9878F8FF, 0xF878F8FF, 0xF85898FF, 0xF87858FF, 0xFCA044FF,
  0xF8B800FF, 0xB8F818FF, 0x58D854FF, 0x58F898FF, 0x00E8D8FF, 0x787878FF, 0x000000FF, 0x000000FF,
  0xFCFCFCFF, 0xA4E4FCFF, 0xB8B8F8FF, 0xD8B8F8FF, 0xF8B8F8FF, 0xF8A4C0FF, 0xF0D0B0FF, 0xFCE0A8FF,
  0xF8D878FF, 0xD8F878FF, 0xB8F8B8FF, 0xB8F8D8FF, 0x00FCFCFF, 0xF8D8F8FF, 0x000000FF, 0x000000FF,
};



SDLVideoDevice::SDLVideoDevice():
  window(
    SDL_CreateWindow(
      "",
      SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
      1024,480,
      SDL_WINDOW_OPENGL
    )
  ),
  glcon(
    new SDL_GLContext(SDL_GL_CreateContext(window))
  )
{

  glMatrixMode(GL_PROJECTION|GL_MODELVIEW);
  glLoadIdentity();
  //glOrtho(0,256,240,0,0,1);
  glOrtho(0,512,240,0,0,1);
  glClearColor(0,0.6,0,1);
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  for (auto& i : _buffer2) {
    i = 0;
  }
  glTexImage2D(
    GL_TEXTURE_2D, 
    0, 
    GL_RGBA, 
    256,
    240,
    0,
    GL_RGBA,
    GL_UNSIGNED_INT_8_8_8_8, 
    _buffer2.data()
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


void SDLVideoDevice::put_pixel(uint8_t x, uint8_t y, PaletteIndex i) {
  _buffer.at(y * 256 + (x % 256)) = RGB[i];
}


void SDLVideoDevice::on_frame() {
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    256,
    240,
    0,
    GL_RGBA,
    GL_UNSIGNED_INT_8_8_8_8, 
    static_cast<const GLvoid*>(_buffer.data())
  );
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(0.0, 0.0f);  glVertex2i(0,0);
  glTexCoord2f(1.0, 0.0f);  glVertex2i(256,0);
  glTexCoord2f(0.0, 1.0f);  glVertex2i(0,240);
  glTexCoord2f(1.0, 1.0f);  glVertex2i(256,240);
  glEnd();

  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    256,
    240,
    0,
    GL_RGBA,
    GL_UNSIGNED_INT_8_8_8_8, 
    static_cast<const GLvoid*>(_buffer2.data())
  );
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(0.0, 0.0f);  glVertex2i(256,0);
  glTexCoord2f(1.0, 0.0f);  glVertex2i(512,0);
  glTexCoord2f(0.0, 1.0f);  glVertex2i(256,240);
  glTexCoord2f(1.0, 1.0f);  glVertex2i(512,240);
  glEnd();

  SDL_GL_SwapWindow(window);
  glClear(GL_COLOR_BUFFER_BIT);

  for (auto& c : _buffer2) {
    c *= 0.7;
    
  }
}



void SDLVideoDevice::on_change(observable<uint16_t> const* pc, uint16_t was, uint16_t is) {
  _buffer2[was & 0xefff] -= 0x10000000;
  _buffer2[is & 0xefff] = 0xff0000ff;
}

void SDLVideoDevice::on_change(observable<uint8_t> const* p, uint8_t was, uint8_t is) {
  static size_t a = 0;
  if (!a) {
    a = (size_t)p;
  }

  auto c = (int)((is / 255.0) * 0xffffff00) | 0xff;
  auto i = ((size_t)p - a) / sizeof(observable<uint8_t>);
  i *= 4;

  for (int j = 0; j < 4; ++j) {
    for (int k = 0; k < 4; ++k) {
      _buffer2[i + 256 * (i / 256) * 3 + 256 * j + k] = c;
    }
  }

}



