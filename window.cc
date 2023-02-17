#include "window.h"
#include "context.h"
#include "event.h"
#include <iostream>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

static void errorCallback(int error, const char *note) { fprintf(stderr, "error: %s\n", note); }

static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    auto context = (Context *)glfwGetWindowUserPointer(window);
    event_fire(context->eventSystemState, EVENT_CODE_QUIT, context, {});
  }
}

static void windowShouldCloseCallback(GLFWwindow *window) {
  auto context = (Context *)glfwGetWindowUserPointer(window);
  event_fire(context->eventSystemState, EVENT_CODE_QUIT, context, {});
}

static void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  auto context = (Context *)glfwGetWindowUserPointer(window);
  EventContext eventContext{};
  eventContext.u32[0] = width;
  eventContext.u32[1] = height;
  event_fire(context->eventSystemState, EVENT_CODE_RESIZED, context, eventContext);
}

static void dropCallback(GLFWwindow *window, int pathCount, const char *paths[]) {
  for (uint i = 0; i < pathCount; ++i) {
    fprintf(stdout, "drop: %s\n", paths[i]);
  }
}

bool window_create(Window **window, u16 width, u16 height, void *pointer) {
  glfwSetErrorCallback(errorCallback);
  if (!glfwInit()) { return false; }

  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  auto handle = glfwCreateWindow(width, height, "neon", nullptr, nullptr);
  if (!handle) {
    glfwTerminate();
    return false;
  }

  auto w = new Window();
  w->handle = handle;
  w->width = width;
  w->height = height;
  *window = w;

  glfwSetKeyCallback(handle, keyCallback);
  glfwSetWindowCloseCallback(handle, windowShouldCloseCallback);
  glfwSetFramebufferSizeCallback(handle, framebufferSizeCallback);
  glfwSetDropCallback(handle, dropCallback);
  glfwSetWindowUserPointer(handle, pointer);

  glfwMakeContextCurrent(handle);

  std::cout << "OpenGL Vendor:" << glGetString(GL_VENDOR) << std::endl;
  std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GLSL Version:" << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  glfwSwapInterval(1);
  return true;
}

void window_destroy(Window **window) {
  glfwDestroyWindow((*window)->handle);
  glfwTerminate();
  DELETE(*window)
}

bool window_should_close(Window *window) { return glfwWindowShouldClose(window->handle); }

void window_poll_events(Window *window) { glfwPollEvents(); }

void window_swap_buffers(Window *window) { glfwSwapBuffers(window->handle); }

f64 window_get_time() { return glfwGetTime(); }
