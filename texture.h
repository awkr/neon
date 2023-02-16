#pragma once

#include "defines.h"
#include <OpenGL/gl3.h>

struct Texture {
  GLuint id;
  u32 width;
  u32 height;
};

bool texture_create(Texture **texture, const char *filepath);
void texture_destroy(Texture **texture);
void texture_bind(Texture *texture, u8 slot = 0);
