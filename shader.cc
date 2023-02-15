#include "shader.h"
#include "filesystem.h"
#include <cstdio>
#include <cstdlib>

bool shader_create(GLuint *shader, GLuint type, const char *path);
void shader_destroy(GLuint shader);

bool program_create(GLuint *program, const std::vector<GLuint> &shaders);

bool shader_create(GLuint *shader, GLuint type, const char *path) {
  // Read from file
  File file{};
  if (!filesystem_open(path, FILE_MODE_READ, false, &file)) { return false; }
  u64 size = 0;
  if (!filesystem_size(&file, &size)) { return false; }
  auto buffer = (GLchar *)calloc(size, sizeof(GLchar));
  u64 read = 0;
  if (!filesystem_read(&file, buffer, &read)) {
    free(buffer);
    return false;
  }

  // Create shader
  auto handle = glCreateShader(type);
  glShaderSource(handle, 1, &buffer, nullptr);
  glCompileShader(handle);
  GLint compiled;
  glGetShaderiv(handle, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    GLsizei len;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &len);

    auto log = new GLchar[len];
    glGetShaderInfoLog(handle, len, &len, log);
    fprintf(stderr, "shader compilation failed: %s\n", log);
    delete[] log;
  }
  *shader = handle;

  free(buffer);
  return true;
}

void shader_destroy(GLuint shader) { glDeleteShader(shader); }

bool program_create(GLuint *program, const std::vector<std::pair<GLuint, const char *>> &files) {
  std::vector<GLuint> shaders;
  for (const auto &it : files) {
    const auto &type = it.first;
    const auto &path = it.second;
    GLuint shader;
    if (!shader_create(&shader, type, path)) { return false; }
    shaders.emplace_back(shader);
  }
  return program_create(program, shaders);
}

void program_use(GLuint program) { glUseProgram(program); }

void program_destroy(GLuint program) { glDeleteProgram(program); }

bool program_create(GLuint *program, const std::vector<GLuint> &shaders) {
  auto handle = glCreateProgram();
  for (const auto &it : shaders) {
    glAttachShader(handle, it);
  }
  glLinkProgram(handle);
  GLint linked;
  glGetProgramiv(handle, GL_LINK_STATUS, &linked);
  if (!linked) {
    GLsizei len;
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &len);

    auto log = new GLchar[len];
    glGetProgramInfoLog(handle, len, &len, log);
    fprintf(stderr, "program linking failed: %s\n", log);
    delete[] log;
    return false;
  }
  for (const auto &it : shaders) {
    shader_destroy(it);
  }
  *program = handle;
  return true;
}