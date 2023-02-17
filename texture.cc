#include "texture.h"
#include <stb_image.h>

bool texture_create(Texture **texture, const char *filepath) {
  stbi_set_flip_vertically_on_load(true);
  int width, height, channels;
  auto buffer = stbi_load(filepath, &width, &height, &channels, 0);
  if (!buffer) { return false; }

  GLuint id;
  glGenTextures(1, &id);

  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  GLint internalFormat = GL_RGBA;
  GLenum format = GL_RGBA;
  if (channels == 3) {
    internalFormat = GL_RGB;
    format = GL_RGB;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Default is 4
  }
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE,
               buffer);
  glGenerateMipmap(GL_TEXTURE_2D);

  if (channels == 3) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // Restore
  }

  stbi_image_free(buffer);

  auto handle = new Texture();
  handle->id = id;
  handle->width = width;
  handle->height = height;
  *texture = handle;
  return true;
}

void texture_destroy(Texture **texture) {
  glDeleteTextures(1, &(*texture)->id);
  DELETE(*texture);
}

void texture_bind(Texture *texture, u8 slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, texture->id);
}
