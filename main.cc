// #include "context.h"
// #include "event.h"
// #include "renderer.h"
// #include "window.h"
//
// bool event_on_quit(EventContext context, void *sender);
// bool event_on_resized(EventContext context, void *sender);
//
// void shutdown(Context *context) {
//   renderer_system_shutdown(&context->renderSystemState);
//   event_deregister(context->eventSystemState, EVENT_CODE_RESIZED, nullptr, event_on_resized);
//   event_deregister(context->eventSystemState, EVENT_CODE_QUIT, nullptr, event_on_quit);
//   event_system_shutdown(&context->eventSystemState);
//   window_destroy(&context->window);
// }
//
// int main() {
//   Context context{};
//
//   if (!window_create(&context.window, 640, 480, &context)) { return EXIT_FAILURE; }
//
//   event_system_initialize(&context.eventSystemState);
//
//   event_register(context.eventSystemState, EVENT_CODE_QUIT, nullptr, event_on_quit);
//   event_register(context.eventSystemState, EVENT_CODE_RESIZED, nullptr, event_on_resized);
//
//   renderer_system_startup(&context.renderSystemState, &context);
//
//   while (!context.quit) {
//     window_poll_events(context.window);
//   }
//
//   shutdown(&context);
//
//   return EXIT_SUCCESS;
// }
//
// bool event_on_quit(EventContext context, void *sender) {
//   ((Context *)sender)->quit = true;
//   return true;
// }
//
// bool event_on_resized(EventContext context, void *sender) {
//   // ((Context *)sender)->window->width = context.u32[0];
//   // ((Context *)sender)->window->height = context.u32[1];
//
//   // display((Context *)sender);
//
//   return true;
// }

#include "program.h"
#include "texture.h"
#include <SDL.h>
#include <cassert>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <pthread.h>

using Clock = std::chrono::steady_clock;

struct Context {
  bool quit;
  SDL_Window *window;
};

enum { VAO_TRIANGLE, VAO_COUNT };

enum { VBO_TRIANGLE, VBO_COUNT };

enum { EBO_TRIANGLE, EBO_COUNT };

enum {
  vPosition = 0,
  vTexCoord = 1,
};

GLuint VAOs[VAO_COUNT];
GLuint VBOs[VBO_COUNT];
GLuint EBOs[VBO_COUNT];
const GLuint kNumVertices = 4;
GLuint program;
Texture *texture;

struct Vertex {
  f32 position[2];
  f32 texCoord[2];
};

void init() {
  glGenVertexArrays(VAO_COUNT, VAOs);
  glBindVertexArray(VAOs[VAO_TRIANGLE]);

  Vertex vertices[kNumVertices] = {
      {{-0.5f, -0.5f}, {0.0, 0.0}}, // Left bottom
      {{0.5f, -0.5f}, {1.0, 0.0}},  // Right bottom
      {{-0.5f, 0.5f}, {0.0, 1.0}},  // Left top
      {{0.5f, 0.5f}, {1.0, 1.0}},   // Right top
  };

  glGenBuffers(VBO_COUNT, VBOs);
  glBindBuffer(GL_ARRAY_BUFFER, VBOs[VBO_TRIANGLE]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  u32 indices[6] = {
      0, 1, 2, // Triangle 1
      1, 3, 2, // Triangle 2
  };

  glGenBuffers(EBO_COUNT, EBOs);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[EBO_TRIANGLE]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  auto ok = texture_create(&texture, "container.jpg");
  assert(ok);

  ok = program_create(&program,
                      {{GL_VERTEX_SHADER, "shader.vert"}, {GL_FRAGMENT_SHADER, "shader.frag"}});
  assert(ok);

  program_use(program);

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

void render(u32 width, u32 height) {
  glViewport(0, 0, width, height);
  // glEnable(GL_CULL_FACE);
  // glFrontFace(GL_CCW);
  // glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(VAOs[VAO_TRIANGLE]);
  program_use(program);
  texture_bind(texture);

  {
    glm::mat4 model(1.0);
    model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(1.0, 0.0, 0.0));

    glm::mat4 view(1.0);
    // view = glm::translate(view, glm::vec3(0.0, 0.0, -3.0));
    f32 radius = 5.0f;
    auto seconds = (f32)SDL_GetTicks64() / 1000.0f;
    f32 cameraX = sin(seconds) * radius;
    f32 cameraZ = cos(seconds) * radius;
    view = glm::lookAt(glm::vec3(cameraX, 0.0f, cameraZ), glm::vec3(0.0f, 0.0f, 0.0),
                       glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 projection(1.0);
    projection = glm::perspective(glm::radians(45.0f), (f32)width / (f32)height, 0.1f, 100.0f);

    program_set_mat4f(program, "model", glm::value_ptr(model));
    program_set_mat4f(program, "view", glm::value_ptr(view));
    program_set_mat4f(program, "projection", glm::value_ptr(projection));
  }

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  glFlush();
}

void *render_thread_main(void *args) {
  auto context = (Context *)args;
  auto glContext = SDL_GL_CreateContext(context->window);
  SDL_GL_MakeCurrent(context->window, glContext);
  init();
  auto start = Clock::now();
  while (!context->quit) {
    auto now = Clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
    int w, h;
    SDL_GL_GetDrawableSize(context->window, &w, &h);
    render(w, h);
    SDL_GL_SwapWindow(context->window);
  }
  pthread_exit(nullptr);
}

int main(int argc, char **argv) {
  Context context{};
  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    fprintf(stderr, "error initializing SDL: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  auto window = SDL_CreateWindow("neon", 0, 0, 640, 480,
                                 SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE |
                                     SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  if (!window) {
    fprintf(stderr, "error creating window: %s\n", SDL_GetError());
    SDL_Quit();
    return EXIT_FAILURE;
  }
  context.window = window;
  pthread_t renderThread;
  pthread_create(&renderThread, nullptr, render_thread_main, &context);
  SDL_Event event;
  while (!context.quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: {
        context.quit = true;
        break;
      }
      case SDL_KEYUP:
        switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_ESCAPE: {
          context.quit = true;
          break;
        }
        default: break;
        }
        break;
      default: break;
      }
    }
    if (context.quit) { break; }
  }
  pthread_join(renderThread, nullptr);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
