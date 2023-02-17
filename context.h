#pragma once

#include <OpenGL/gl3.h>

struct Context {
  bool quit;
  struct Window *window;
  void *eventSystemState;
  GLuint program;
  struct Texture *texture;
};
