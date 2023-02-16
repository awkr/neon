#include "window.h"
#include <iostream>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

static void errorCallback(int error, const char *note) { fprintf(stderr, "error: %s\n", note); }

static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

static void dropCallback(GLFWwindow *window, int pathCount, const char *paths[]) {
  for (uint i = 0; i < pathCount; ++i) {
    fprintf(stdout, "drop: %s\n", paths[i]);
  }
}

bool window_create(GLFWwindow **window) {
  glfwSetErrorCallback(errorCallback);
  if (!glfwInit()) { return false; }

  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  auto handle = glfwCreateWindow(640, 480, "neon", nullptr, nullptr);
  if (!handle) {
    glfwTerminate();
    return false;
  }
  *window = handle;

  glfwSetKeyCallback(handle, keyCallback);
  glfwSetDropCallback(handle, dropCallback);

  glfwMakeContextCurrent(handle);

  std::cout << "OpenGL Vendor:" << glGetString(GL_VENDOR) << std::endl;
  std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GLSL Version:" << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  glfwSwapInterval(1);
  return true;
}

void window_destroy(GLFWwindow *window) {
  glfwDestroyWindow(window);
  glfwTerminate();
}

bool window_should_close(GLFWwindow *window) { return glfwWindowShouldClose(window); }

void window_poll_events(GLFWwindow *window) { glfwPollEvents(); }

void window_swap_buffers(GLFWwindow *window) { glfwSwapBuffers(window); }

f64 window_get_time() { return glfwGetTime(); }
