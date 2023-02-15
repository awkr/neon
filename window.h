#pragma once

struct Context;
struct GLFWwindow;

bool window_create(Context *context);
void window_destroy(GLFWwindow *window);
bool window_should_close(GLFWwindow *window);
void window_poll_events(GLFWwindow *window);
void window_swap_buffers(GLFWwindow *window);
