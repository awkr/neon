#include <iostream>

#include "context.h"
#include "defines.h"
#include "shader.h"
#include "window.h"

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

  auto ok = program_create(
      &context->program, {{GL_VERTEX_SHADER, "shader.vert"}, {GL_FRAGMENT_SHADER, "shader.frag"}});
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

  if (!window_create(&context.window)) { return EXIT_FAILURE; }

  init(&context);

  while (!window_should_close(context.window)) {
    display(&context);
    window_swap_buffers(context.window);
    window_poll_events(context.window);
  }

  shutdown(&context);

  return EXIT_SUCCESS;
}
