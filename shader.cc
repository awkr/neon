#include "shader.h"
#include "filesystem.h"
#include <cstdio>
#include <cstdlib>

bool shader_create(GLuint type, const char *path, GLuint *outShader);
void shader_destroy(GLuint shader);

bool program_create(const std::vector<GLuint> &shaders, GLuint *outProgram);

bool shader_create(GLuint type, const char *path, GLuint *outShader) {
  // Read from file
  File file{};
  if (!filesystem_open(path, FILE_MODE_READ, false, &file)) {
    return false;
  }
  u64 size = 0;
  if (!filesystem_size(&file, &size)) {
    return false;
  }
  auto buffer = (GLchar *)calloc(size, sizeof(GLchar));
  u64 read = 0;
  if (!filesystem_read(&file, buffer, &read)) {
    free(buffer);
    return false;
  }

  // Create shader
  auto shader = glCreateShader(type);
  glShaderSource(shader, 1, &buffer, nullptr);
  glCompileShader(shader);
  GLint compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    GLsizei len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

    auto log = new GLchar[len];
    glGetShaderInfoLog(shader, len, &len, log);
    fprintf(stderr, "shader compilation failed: %s\n", log);
    delete[] log;
  }
  *outShader = shader;

  free(buffer);
  return true;
}

void shader_destroy(GLuint shader) { glDeleteShader(shader); }

bool program_create(const std::vector<std::pair<GLuint, const char *>> &files, GLuint *outProgram) {
  std::vector<GLuint> shaders;
  for (const auto &it : files) {
    const auto &type = it.first;
    const auto &path = it.second;
    GLuint shader;
    if (!shader_create(type, path, &shader)) {
      return false;
    }
    shaders.emplace_back(shader);
  }
  return program_create(shaders, outProgram);
}

void program_use(GLuint program) { glUseProgram(program); }

void program_destroy(GLuint program) { glDeleteProgram(program); }

bool program_create(const std::vector<GLuint> &shaders, GLuint *outProgram) {
  auto program = glCreateProgram();
  for (const auto &it : shaders) {
    glAttachShader(program, it);
  }
  glLinkProgram(program);
  GLint linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (!linked) {
    GLsizei len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

    auto log = new GLchar[len];
    glGetProgramInfoLog(program, len, &len, log);
    fprintf(stderr, "program linking failed: %s\n", log);
    delete[] log;
    return false;
  }
  for (const auto &it : shaders) {
    shader_destroy(it);
  }
  *outProgram = program;
  return true;
}
