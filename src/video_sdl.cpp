#include "video_sdl.h"
#include "SDL.h"

#include <iostream>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <gl.h>
#include <glext.h>

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


void SDLVideoDevice::set_buffer(Framebuffer const& buffer) {
  _buffer = &buffer;
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
    static_cast<const GLvoid*>(_buffer->data())
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
    c *= 0.6;
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



