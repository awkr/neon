#pragma once

#include <OpenGL/gl3.h>
#include <utility>
#include <vector>

bool program_create(GLuint *program, const std::vector<std::pair<GLuint, const char *>> &files);
void program_use(GLuint program);
void program_destroy(GLuint program);
