#include <cstdio>
#include <iostream>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#define BUFFER_OFFSET(a) ((void *)(a))

static void errorCallback(int error, const char *note) { fprintf(stderr, "error: %s\n", note); }

static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

enum { Triangle, NumVao };

enum { VertexBuffer, NumBuffer };

enum {
  vPosition = 0,
};

GLuint vaos[NumVao];
GLuint buffers[NumBuffer];
const GLuint kNumVertices = 6;
GLuint ebo;

GLuint makeShader(GLuint sType, const GLchar *shaderStr) {
  GLuint shader = glCreateShader(sType);
  glShaderSource(shader, 1, &shaderStr, nullptr);
  glCompileShader(shader);
  GLint compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    GLsizei len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

    GLchar *log = new GLchar[len];
    glGetShaderInfoLog(shader, len, &len, log);
    std::cerr << "shader compilation failed: " << log << std::endl;
    delete[] log;
    return 0;
  }
  return shader;
}

void init() {
  glGenVertexArrays(NumVao, vaos);
  glBindVertexArray(vaos[Triangle]);

  GLfloat vertices[kNumVertices][2] = {{-0.90, -0.90}, {0.85, -0.90}, {-0.90, 0.85},
                                       {0.90, -0.85},  {0.90, 0.90},  {-0.85, 0.90}};

  glGenBuffers(NumBuffer, buffers);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[VertexBuffer]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  uint indices[6] = {// Triangle 1
                     0, 1, 2,
                     // Triangle 2
                     3, 4, 5};

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  const GLchar *vertShaderStr = R"(
#version 410 core
layout( location = 0) in vec4 vPosition;
void main() {
  gl_Position = vPosition;
}
)";

  GLuint vertShader = makeShader(GL_VERTEX_SHADER, vertShaderStr);

  const GLchar *fragShaderStr = R"(
#version 410 core
out vec4 fColor;
void main() {
  fColor = vec4(0.5, 0.4, 0.8, 1.0);
}
)";
  GLuint fragShader = makeShader(GL_FRAGMENT_SHADER, fragShaderStr);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertShader);
  glAttachShader(program, fragShader);
  glLinkProgram(program);
  GLint linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (!linked) {
    GLsizei len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

    GLchar *log = new GLchar[len];
    glGetProgramInfoLog(program, len, &len, log);
    std::cerr << "shader linking failed: " << log << std::endl;
    delete[] log;
  }
  glDeleteShader(fragShader);
  glDeleteShader(vertShader);

  glUseProgram(program);

  glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), BUFFER_OFFSET(0));
  glEnableVertexAttribArray(vPosition);

  // The call to glVertexAttribPointer already registered `VBO_TRIANGLE` as the vertex attribute's
  // bound vertex buffer object
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(vaos[Triangle]);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  glFlush();
}

int main() {
  glfwSetErrorCallback(errorCallback);
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  GLFWwindow *window = glfwCreateWindow(640, 480, "neon", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, keyCallback);

  glfwMakeContextCurrent(window);

  std::cout << "OpenGL Vendor:" << glGetString(GL_VENDOR) << std::endl;
  std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GLSL Version:" << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  glfwSwapInterval(1);

  init();

  while (!glfwWindowShouldClose(window)) {
    display();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}
