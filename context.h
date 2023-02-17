#pragma once

#include <OpenGL/gl3.h>

struct Context {
  struct Window *window;
  GLuint program;
  struct Texture *texture;
};
