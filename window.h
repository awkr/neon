#pragma once

#include "defines.h"

struct Window {
  struct GLFWwindow *handle;
  u16 width;
  u16 height;
};

bool window_create(Window **ppWindow, u16 width, u16 height, void *userPtr = nullptr);
void window_destroy(Window **window);
bool window_should_close(Window *window);
void window_poll_events(Window *window);
void window_swap_buffers(Window *window);
void window_make_context_current(Window *window);
void window_set_swap_interval(Window *window, u8 interval);
f32 window_get_time();
