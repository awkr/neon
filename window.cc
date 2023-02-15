#include "window.h"
#include <iostream>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include "context.h"

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

bool window_create(Context *context) {
  glfwSetErrorCallback(errorCallback);
  if (!glfwInit()) { return false; }

  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  auto window = glfwCreateWindow(640, 480, "neon", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return false;
  }
  context->window = window;

  glfwSetKeyCallback(window, keyCallback);
  glfwSetDropCallback(window, dropCallback);

  glfwMakeContextCurrent(window);

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
