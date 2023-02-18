#pragma once

#include <pthread.h>

struct Vertex {
  float position[2];
  float texCoord[2];
};

bool renderer_system_startup(void **state, void *userPtr);
void renderer_system_shutdown(void **state);
