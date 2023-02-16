#pragma once

#include "defines.h"
#include <OpenGL/gl3.h>
#include <vector>

bool program_create(GLuint *program, const std::vector<std::pair<GLuint, const char *>> &files);
void program_use(GLuint program);
void program_destroy(GLuint program);
GLint program_get_uniform_location(GLuint program, const char *name);
void program_set_i32(GLuint program, const char *name, i32 a);
void program_set_f32(GLuint program, const char *name, f32 a);
void program_set_bool(GLuint program, const char *name, bool a);
void program_set_mat4f(GLuint program, const char *name, const GLfloat *a);
