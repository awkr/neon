#include "context.h"
#include "shader.h"
#include "texture.h"
#include "window.h"
#include <iostream>

enum { VAO_TRIANGLE, VAO_COUNT };

enum { VBO_TRIANGLE, VBO_COUNT };

enum { EBO_TRIANGLE, EBO_COUNT };

enum {
  vPosition = 0,
  vTexCoord = 1,
};

GLuint vaos[VAO_COUNT];
GLuint buffers[VBO_COUNT];
GLuint ebos[VBO_COUNT];
const GLuint kNumVertices = 4;

struct Vertex {
  float position[2];
  float texCoord[2];
};

void init(Context *context) {
  glGenVertexArrays(VAO_COUNT, vaos);
  glBindVertexArray(vaos[VAO_TRIANGLE]);

  Vertex vertices[kNumVertices] = {
      {{-0.90, -0.90}, {0.0, 0.0}}, // Left bottom
      {{0.90, -0.90}, {1.0, 0.0}},  // Right bottom
      {{-0.90, 0.90}, {0.0, 1.0}},  // Left top
      {{0.90, 0.90}, {1.0, 1.0}},   // Right top
  };

  glGenBuffers(VBO_COUNT, buffers);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[VBO_TRIANGLE]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  uint indices[6] = {
      0, 1, 2, // Triangle 1
      1, 3, 2, // Triangle 2
  };

  glGenBuffers(EBO_COUNT, ebos);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[EBO_TRIANGLE]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  auto ok = texture_create(&context->texture, "container.jpg");
  assert(ok);

  ok = program_create(&context->program,
                      {{GL_VERTEX_SHADER, "shader.vert"}, {GL_FRAGMENT_SHADER, "shader.frag"}});
  assert(ok);

  program_use(context->program);

  glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (any)offsetof(Vertex, position));
  glEnableVertexAttribArray(vPosition);

  glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (any)offsetof(Vertex, texCoord));
  glEnableVertexAttribArray(vTexCoord);

  // The call to glVertexAttribPointer already registered `VBO_TRIANGLE` as the vertex attribute's
  // bound vertex buffer object
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void shutdown(Context *context) {
  program_destroy(context->program);
  texture_destroy(&context->texture);
  window_destroy(context->window);
}

void display(Context *context) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(vaos[VAO_TRIANGLE]);
  program_use(context->program);
  texture_bind(context->texture, 0);
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
