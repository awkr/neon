#include <cstdio>
#include <iostream>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include "shader.h"

#define BUFFER_OFFSET(a) ((void *)(a))

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

enum { VAO_TRIANGLE, VAO_COUNT };

enum { VBO_TRIANGLE, VBO_COUNT };

enum { EBO_TRIANGLE, EBO_COUNT };

enum {
  vPosition = 0,
};

GLuint vaos[VAO_COUNT];
GLuint buffers[VBO_COUNT];
GLuint ebos[VBO_COUNT];
const GLuint kNumVertices = 6;

struct Context {
  GLFWwindow *window;
  GLuint program;
};

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

void init(Context *context) {
  glGenVertexArrays(VAO_COUNT, vaos);
  glBindVertexArray(vaos[VAO_TRIANGLE]);

  GLfloat vertices[kNumVertices][2] = {{-0.90, -0.90}, {0.85, -0.90}, {-0.90, 0.85},
                                       {0.90, -0.85},  {0.90, 0.90},  {-0.85, 0.90}};

  glGenBuffers(VBO_COUNT, buffers);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[VBO_TRIANGLE]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  uint indices[6] = {// Triangle 1
                     0, 1, 2,
                     // Triangle 2
                     3, 4, 5};

  glGenBuffers(EBO_COUNT, ebos);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[EBO_TRIANGLE]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  auto ok = program_create({{GL_VERTEX_SHADER, "shader.vert"}, {GL_FRAGMENT_SHADER, "shader.frag"}},
                           &context->program);
  assert(ok);

  program_use(context->program);

  glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), BUFFER_OFFSET(0));
  glEnableVertexAttribArray(vPosition);

  // The call to glVertexAttribPointer already registered `VBO_TRIANGLE` as the vertex attribute's
  // bound vertex buffer object
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void shutdown(Context *context) {
  program_destroy(context->program);
  window_destroy(context->window);
}

void display(Context *context) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(vaos[VAO_TRIANGLE]);
  program_use(context->program);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  glFlush();
}

int main() {
  Context context{};

  if (!window_create(&context)) { return EXIT_FAILURE; }

  init(&context);

  while (!glfwWindowShouldClose(context.window)) {
    display(&context);
    glfwSwapBuffers(context.window);
    glfwPollEvents();
  }

  shutdown(&context);

  return EXIT_SUCCESS;
}
