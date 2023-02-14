#pragma once

#include <OpenGL/gl3.h>
#include <utility>
#include <vector>

bool program_create(const std::vector<std::pair<GLuint, const char *>> &files, GLuint *outProgram);
void program_use(GLuint program);
void program_destroy(GLuint program);
