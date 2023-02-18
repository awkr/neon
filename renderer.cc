#include "renderer.h"
#include "context.h"
#include "defines.h"
#include "program.h"
#include "texture.h"
#include "window.h"
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

struct RendererSystemState {
  pthread_t renderThread;
  GLuint program;
  Texture *texture;
};

void init(RendererSystemState *state) {
  glGenVertexArrays(VAO_COUNT, vaos);
  glBindVertexArray(vaos[VAO_TRIANGLE]);

  Vertex vertices[kNumVertices] = {
      {{-0.5f, -0.5f}, {0.0, 0.0}}, // Left bottom
      {{0.5f, -0.5f}, {1.0, 0.0}},  // Right bottom
      {{-0.5f, 0.5f}, {0.0, 1.0}},  // Left top
      {{0.5f, 0.5f}, {1.0, 1.0}},   // Right top
  };

  glGenBuffers(VBO_COUNT, buffers);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[VBO_TRIANGLE]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  u32 indices[6] = {
      0, 1, 2, // Triangle 1
      1, 3, 2, // Triangle 2
  };

  glGenBuffers(EBO_COUNT, ebos);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[EBO_TRIANGLE]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  auto ok = texture_create(&state->texture, "container.jpg");
  assert(ok);

  ok = program_create(&state->program,
                      {{GL_VERTEX_SHADER, "shader.vert"}, {GL_FRAGMENT_SHADER, "shader.frag"}});
  assert(ok);

  program_use(state->program);

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

void display(RendererSystemState *state) {
  // glViewport(0, 0, state->window->width, state->window->height);
  // glEnable(GL_CULL_FACE);
  // glFrontFace(GL_CCW);
  // glCullFace(GL_BACK);
  // glEnable(GL_DEPTH_TEST);
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // glClearColor(1.0, 0.0, 0.0, 1.0);
  // glBindVertexArray(vaos[VAO_TRIANGLE]);
  // program_use(state->program);
  // texture_bind(state->texture, 0);
  //
  // {
  //   glm::mat4 model(1.0);
  //   model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(1.0, 0.0, 0.0));
  //
  //   glm::mat4 view(1.0);
  //   // view = glm::translate(view, glm::vec3(0.0, 0.0, -3.0));
  //   float radius = 5.0f;
  //   float cameraX = sin(window_get_time()) * radius;
  //   float cameraZ = cos(window_get_time()) * radius;
  //   view = glm::lookAt(glm::vec3(cameraX, 0.0f, cameraZ), glm::vec3(0.0f, 0.0f, 0.0),
  //                      glm::vec3(0.0f, 1.0f, 0.0f));
  //
  //   glm::mat4 projection(1.0);
  //   projection = glm::perspective(
  //       glm::radians(45.0f), (f32)state->window->width / (f32)state->window->height, 0.1f,
  //       100.0f);
  //
  //   program_set_mat4f(state->program, "model", glm::value_ptr(model));
  //   program_set_mat4f(state->program, "view", glm::value_ptr(view));
  //   program_set_mat4f(state->program, "projection", glm::value_ptr(projection));
  // }
  //
  // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  // glFlush();
  //
  // window_swap_buffers(state->window);
}

void *render(void *arg) {
  auto context = (Context *)arg;

  window_make_context_current(context->window);

  printf("OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
  printf("OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
  printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
  printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

  window_set_swap_interval(context->window, 1);

  init((RendererSystemState *)context->renderSystemState);

  while (!context->quit) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glFlush();

    auto state = (RendererSystemState *)context->renderSystemState;
    {
      // glViewport(0, 0, context->window->width, context->window->height);
      // glEnable(GL_CULL_FACE);
      // glFrontFace(GL_CCW);
      // glCullFace(GL_BACK);
      // glEnable(GL_DEPTH_TEST);
      // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      // glClearColor(1.0, 0.0, 0.0, 1.0);
      // glBindVertexArray(vaos[VAO_TRIANGLE]);
      // program_use(state->program);
      // texture_bind(state->texture, 0);
      //
      // {
      //   glm::mat4 model(1.0);
      //   model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(1.0, 0.0, 0.0));
      //
      //   glm::mat4 view(1.0);
      //   view = glm::translate(view, glm::vec3(0.0, 0.0, -3.0));
      //   // float radius = 5.0f;
      //   // float cameraX = sin(window_get_time()) * radius;
      //   // float cameraZ = cos(window_get_time()) * radius;
      //   // view = glm::lookAt(glm::vec3(cameraX, 0.0f, cameraZ), glm::vec3(0.0f, 0.0f, 0.0),
      //   //                    glm::vec3(0.0f, 1.0f, 0.0f));
      //
      //   glm::mat4 projection(1.0);
      //   projection = glm::perspective(glm::radians(45.0f),
      //                                 (f32)context->window->width / (f32)context->window->height,
      //                                 0.1f, 100.0f);
      //
      //   program_set_mat4f(state->program, "model", glm::value_ptr(model));
      //   program_set_mat4f(state->program, "view", glm::value_ptr(view));
      //   program_set_mat4f(state->program, "projection", glm::value_ptr(projection));
      // }
      //
      // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
      // glFlush();
    }

    window_swap_buffers(context->window);
  }

  // glfwMakeContextCurrent(NULL);

  pthread_exit(nullptr);
}

bool renderer_system_startup(void **state, void *userPtr) {
  pthread_t renderThread;
  if (pthread_create(&renderThread, nullptr, render, userPtr)) { return false; }
  auto s = new RendererSystemState();
  s->renderThread = renderThread;
  // init(s);
  *state = s;
  return true;
}

void renderer_system_shutdown(void **state) {
  auto s = (RendererSystemState *)(*state);
  pthread_join(s->renderThread, nullptr);
  program_destroy(s->program);
  texture_destroy(&s->texture);
  DELETE(s)
}
