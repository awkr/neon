#pragma once

#include "defines.h"

struct Window {
  struct GLFWwindow *handle;
  u16 width;
  u16 height;
};

bool window_create(Window **window, u16 width, u16 height, void *pointer = nullptr);
void window_destroy(Window **window);
bool window_should_close(Window *window);
void window_poll_events(Window *window);
void window_swap_buffers(Window *window);
f64 window_get_time();
