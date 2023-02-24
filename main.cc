#include "event.h"
#include "input.h"
#include "message_queue.h"
#include "program.h"
#include "texture.h"
#include <SDL.h>
#include <cassert>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <pthread.h>
#include <thread>

using Clock = std::chrono::steady_clock;

static void sleep_for(double d) {
  static constexpr std::chrono::duration<double> minSleepDuration(0);
  Clock::time_point start = Clock::now();
  while (std::chrono::duration<double>(Clock::now() - start).count() < d) {
    std::this_thread::sleep_for(minSleepDuration);
  }
}

struct Context {
  bool quit;
  SDL_Window *window;
  std::unique_ptr<MessageQueue> updateThreadMessageQueue;
  std::unique_ptr<MessageQueue> renderThreadMessageQueue;
  void *eventSystemState;
  void *inputSystemState;
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

enum FrameState {
  FRAME_STATE_EMPTY = 0x0,
  FRAME_STATE_PROCESSING,
  FRAME_STATE_PROCESSED,
  FRAME_STATE_UPDATING,
  FRAME_STATE_UPDATED,
  FRAME_STATE_RENDERING,
  FRAME_STATE_RENDERED,
  FRAME_STATE_PRESENTING,
  FRAME_STATE_PRESENTED,
};

struct Frame {
  FrameState state;
  std::mutex mutex;
  std::condition_variable condition;
};

Frame frames[3] = {};

struct Vertex {
  f32 position[2];
  f32 texCoord[2];
};

bool event_on_quit(EventCode eventCode, EventContext eventContext, void *sender, void *listener);
bool event_on_key(EventCode eventCode, EventContext eventContext, void *sender, void *listener);

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

auto cameraPos = glm::vec3(0.0, 0.0, 3.0);
auto cameraFront = glm::vec3(0.0, 0.0, -1.0);
auto cameraUp = glm::vec3(0.0, 1.0, 0.0);

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

    // f32 radius = 5.0f;
    // auto seconds = (f32)SDL_GetTicks64() / 1000.0f;
    // f32 cameraX = sin(seconds) * radius;
    // f32 cameraZ = cos(seconds) * radius;
    // view = glm::lookAt(glm::vec3(cameraX, 0.0f, cameraZ), glm::vec3(0.0f, 0.0f, 0.0),
    //                    glm::vec3(0.0f, 1.0f, 0.0f));

    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    glm::mat4 projection(1.0);
    projection = glm::perspective(glm::radians(45.0f), (f32)width / (f32)height, 0.1f, 100.0f);

    program_set_mat4f(program, "model", glm::value_ptr(model));
    program_set_mat4f(program, "view", glm::value_ptr(view));
    program_set_mat4f(program, "projection", glm::value_ptr(projection));
  }

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void *update_thread_main(void *args) {
  auto context = (Context *)args;
  auto time = Clock::now();
  bool quit = false;
  while (!quit) {
    Message message;
    if (auto ok = context->updateThreadMessageQueue->pop(&message); !ok) {
      quit = true;
      break;
    }
    switch (message.type) {
    case MESSAGE_TYPE_QUIT: quit = true; break;
    case MESSAGE_TYPE_UPDATE: {
      auto startTime = Clock::now();
      auto deltaTime =
          std::chrono::duration_cast<std::chrono::microseconds>(startTime - time).count();
      time = startTime;

      auto frameIndex = message.u32[0];
      printf("[UpdateThread] update frame #%d, delta time %lld microseconds\n", frameIndex,
             deltaTime);

      // Do some updates

      {
        // Reset camera's transform
        if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_SPACE)) {
          // TODO
        } else {
          // Camera rotation
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_UP)) {}
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_DOWN)) {}
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_LEFT)) {}
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_RIGHT)) {}

          // Camera movement
          auto moveSpeed = 0.001f * deltaTime / 1000.0f;
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_W)) {
            cameraPos += moveSpeed * cameraFront;
          }
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_S)) {
            cameraPos -= moveSpeed * cameraFront;
          }
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_A)) {
            cameraPos -= moveSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
          }
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_D)) {
            cameraPos += moveSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
          }
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_Q)) {
            cameraPos += moveSpeed * cameraUp;
          }
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_E)) {
            cameraPos -= moveSpeed * cameraUp;
          }
        }
      }

      context->renderThreadMessageQueue->push({
          .type = MESSAGE_TYPE_RENDER,
          .u32[0] = frameIndex,
      }); // Issue render commands

      if (frameIndex > 0) { // Wait for previous frame to be presented
        printf("[UpdateThread] frame #%d is waiting for frame #%d to be presented\n", frameIndex,
               frameIndex - 1);
        auto &frame = frames[frameIndex - 1];
        std::unique_lock<std::mutex> lock(frame.mutex);
        frame.condition.wait(lock, [&]() { return frame.state == FRAME_STATE_PRESENTED; });
      }

      auto duration =
          std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - startTime).count();
      auto lifespan = (u64)(1 / 60.0f * 1e9); // Nanoseconds
      if (duration < lifespan) { sleep_for((double)(lifespan - duration) / 1e9); }

      context->updateThreadMessageQueue->push({
          .type = MESSAGE_TYPE_UPDATE,
          .u32[0] = (frameIndex + 1) % 3,
      }); // Start to update next frame
    } break;
    default: break;
    }
  }
  pthread_exit(nullptr);
}

void *render_thread_main(void *args) {
  auto context = (Context *)args;
  auto glContext = SDL_GL_CreateContext(context->window);
  SDL_GL_MakeCurrent(context->window, glContext);
  init();
  bool quit = false;
  while (!quit) {
    Message message;
    if (auto ok = context->renderThreadMessageQueue->pop(&message); !ok) {
      quit = true;
      break;
    }
    switch (message.type) {
    case MESSAGE_TYPE_QUIT: quit = true; break;
    case MESSAGE_TYPE_RENDER: {
      printf("[RenderThread] render frame #%d\n", message.u32[0]);
      int w, h;
      SDL_GL_GetDrawableSize(context->window, &w, &h);
      render(w, h);
      glFinish();
      printf("[RenderThread] about to present frame #%d\n", message.u32[0]);
      SDL_GL_SwapWindow(context->window);
      printf("[RenderThread] frame #%d presented\n", message.u32[0]);
      {
        auto &frame = frames[message.u32[0]];
        std::lock_guard<std::mutex> lock(frame.mutex);
        frame.state = FRAME_STATE_PRESENTED;
        frame.condition.notify_one();
      }
    } break;
    default: break;
    }
  }
  pthread_exit(nullptr);
}

int main(int argc, char **argv) {
  Context context{};
  event_system_initialize(&context.eventSystemState);
  event_register(context.eventSystemState, EVENT_CODE_KEYBOARD_PRESSED, &context, event_on_key);
  event_register(context.eventSystemState, EVENT_CODE_KEYBOARD_RELEASED, &context, event_on_key);
  event_register(context.eventSystemState, EVENT_CODE_QUIT, &context, event_on_quit);
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
  SDL_SetWindowData(window, "EngineContext", &context);
  SDL_AddEventWatch(
      [](void *userdata, SDL_Event *event) -> int {
        if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
          auto window = SDL_GetWindowFromID(event->window.windowID);
          auto context = (Context *)SDL_GetWindowData(window, "EngineContext");
          // context->renderThreadMessageQueue->push({
          //     .type = MESSAGE_TYPE_RENDER,
          // });
        }
        return 0;
      },
      window);
  context.window = window;
  pthread_t renderThread;
  pthread_create(&renderThread, nullptr, render_thread_main, &context);
  context.renderThreadMessageQueue = std::make_unique<MessageQueue>();
  pthread_t updateThread;
  pthread_create(&updateThread, nullptr, update_thread_main, &context);
  context.updateThreadMessageQueue = std::make_unique<MessageQueue>();
  context.updateThreadMessageQueue->push({
      .type = MESSAGE_TYPE_UPDATE,
      .u32[0] = 0, // Starts from frame #0
  });
  input_system_initialize(&context.inputSystemState, context.eventSystemState);
  SDL_Event event;
  while (!context.quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: {
        event_fire(context.eventSystemState, EVENT_CODE_QUIT, nullptr, {});
        break;
      }
      case SDL_KEYDOWN:
      case SDL_KEYUP: {
        input_system_process_key(context.inputSystemState, event.key.keysym.scancode,
                                 event.type == SDL_KEYDOWN);
      } break;
      default: break;
      }
    }
    if (context.quit) { break; }
    input_system_update(context.inputSystemState);
  }
  input_system_shutdown(&context.inputSystemState);
  context.renderThreadMessageQueue->push({
      .type = MESSAGE_TYPE_QUIT,
  }); // Quit render thread
  pthread_join(renderThread, nullptr);
  context.updateThreadMessageQueue->push({
      .type = MESSAGE_TYPE_QUIT,
  }); // Quit update thread
  pthread_join(updateThread, nullptr);
  SDL_DestroyWindow(window);
  SDL_Quit();
  event_deregister(context.eventSystemState, EVENT_CODE_QUIT, &context, event_on_quit);
  event_deregister(context.eventSystemState, EVENT_CODE_KEYBOARD_RELEASED, &context, event_on_key);
  event_deregister(context.eventSystemState, EVENT_CODE_KEYBOARD_PRESSED, &context, event_on_key);
  event_system_shutdown(&context.eventSystemState);
  return EXIT_SUCCESS;
}

bool event_on_quit(EventCode eventCode, EventContext eventContext, void *sender, void *listener) {
  auto context = (Context *)listener;
  context->quit = true;
  return true;
}

bool event_on_key(EventCode eventCode, EventContext eventContext, void *sender, void *listener) {
  if (eventCode == EVENT_CODE_KEYBOARD_RELEASED) {
    if (eventContext.u16[0] == SDL_SCANCODE_ESCAPE) {
      auto context = (Context *)listener;
      event_fire(context->eventSystemState, EVENT_CODE_QUIT, nullptr, {});
    }
  }
  return true;
}
