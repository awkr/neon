#include "camera.h"
#include "event.h"
#include "input.h"
#include "message_queue.h"
#include "program.h"
#include "texture.h"
#include <SDL.h>
#include <cassert>
#include <chrono>
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

enum { VAO_CUBE, VAO_LIGHT, VAO_COUNT };

enum { VBO_CUBE, VBO_LIGHT, VBO_COUNT };

enum { EBO_CUBE, EBO_LIGHT, EBO_COUNT };

enum {
  vPosition = 0,
  vTexCoord = 1,
};

GLuint VAOs[VAO_COUNT];
GLuint VBOs[VBO_COUNT];
GLuint EBOs[VBO_COUNT];
const GLuint kNumVertices = 24;
const GLuint kNumIndices = 36;
GLuint lightingProgram;
GLuint lightCubeProgram;
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
  f32 position[3];
  f32 normal[3];
  f32 texCoord[2];
};

bool event_on_quit(EventCode eventCode, EventContext eventContext, void *sender, void *listener);

bool event_on_key(EventCode eventCode, EventContext eventContext, void *sender, void *listener);

bool event_on_scroll(EventCode eventCode, EventContext eventContext, void *sender, void *listener);

void init() {
  glGenVertexArrays(VAO_COUNT, VAOs);
  glGenBuffers(VBO_COUNT, VBOs);
  glGenBuffers(EBO_COUNT, EBOs);

  // Build and compile shader programs
  {
    auto ok =
        program_create(&lightingProgram, {{GL_VERTEX_SHADER, "shaders/basic_lighting.vert"},
                                          {GL_FRAGMENT_SHADER, "shaders/basic_lighting.frag"}});
    assert(ok);
  }
  {
    auto ok = program_create(&lightCubeProgram, {{GL_VERTEX_SHADER, "shaders/light_cube.vert"},
                                                 {GL_FRAGMENT_SHADER, "shaders/light_cube.frag"}});
    assert(ok);
  }

  { // Cube
    glBindVertexArray(VAOs[VAO_CUBE]);

    Vertex vertices[kNumVertices] = {
        //
        {{-0.5, -0.5, 0.5}, {0, 0, 1}, {0, 0}},
        {{0.5, -0.5, 0.5}, {0, 0, 1}, {1, 0}},
        {{-0.5, 0.5, 0.5}, {0, 0, 1}, {0, 1}},
        {{0.5, 0.5, 0.5}, {0, 0, 1}, {1, 1}},

        //
        {{0.5, -0.5, 0.5}, {1, 0, 0}, {0, 0}},
        {{0.5, -0.5, -0.5}, {1, 0, 0}, {1, 0}},
        {{0.5, 0.5, 0.5}, {1, 0, 0}, {0, 1}},
        {{0.5, 0.5, -0.5}, {1, 0, 0}, {1, 1}},

        //
        {{0.5, -0.5, -0.5}, {0, 0, -1}, {0, 0}},
        {{-0.5, -0.5, -0.5}, {0, 0, -1}, {1, 0}},
        {{0.5, 0.5, -0.5}, {0, 0, -1}, {0, 1}},
        {{-0.5, 0.5, -0.5}, {0, 0, -1}, {1, 1}},

        //
        {{-0.5, -0.5, -0.5}, {-1, 0, 0}, {0, 0}},
        {{-0.5, -0.5, 0.5}, {-1, 0, 0}, {1, 0}},
        {{-0.5, 0.5, -0.5}, {-1, 0, 0}, {0, 1}},
        {{-0.5, 0.5, 0.5}, {-1, 0, 0}, {1, 1}},

        //
        {{-0.5, 0.5, 0.5}, {0, 1, 0}, {0, 0}},
        {{0.5, 0.5, 0.5}, {0, 1, 0}, {1, 0}},
        {{-0.5, 0.5, -0.5}, {0, 1, 0}, {0, 1}},
        {{0.5, 0.5, -0.5}, {0, 1, 0}, {1, 1}},

        //
        {{-0.5, -0.5, -0.5}, {0, -1, 0}, {0, 0}},
        {{0.5, -0.5, -0.5}, {0, -1, 0}, {1, 0}},
        {{-0.5, -0.5, 0.5}, {0, -1, 0}, {0, 1}},
        {{0.5, -0.5, 0.5}, {0, -1, 0}, {1, 1}},
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[VBO_CUBE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    u32 indices[kNumIndices] = {
        0,  1,  2,  1,  3,  2,  //
        4,  5,  6,  5,  7,  6,  //
        8,  9,  10, 9,  11, 10, //
        12, 13, 14, 13, 15, 14, //
        16, 17, 18, 17, 19, 18, //
        20, 21, 22, 21, 23, 22, //
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[EBO_CUBE]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Attribute: position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (any)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    // Attribute: normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (any)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // The call to glVertexAttribPointer already registered the last bound VBO as the vertex
    // attribute's bound vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  { // Light
    glBindVertexArray(VAOs[VAO_LIGHT]);

    Vertex vertices[kNumVertices] = {
        //
        {{-0.5, -0.5, 0.5}, {0, 0}},
        {{0.5, -0.5, 0.5}, {1, 0}},
        {{-0.5, 0.5, 0.5}, {0, 1}},
        {{0.5, 0.5, 0.5}, {1, 1}},

        //
        {{0.5, -0.5, 0.5}, {0, 0}},
        {{0.5, -0.5, -0.5}, {1, 0}},
        {{0.5, 0.5, 0.5}, {0, 1}},
        {{0.5, 0.5, -0.5}, {1, 1}},

        //
        {{0.5, -0.5, -0.5}, {0, 0}},
        {{-0.5, -0.5, -0.5}, {1, 0}},
        {{0.5, 0.5, -0.5}, {0, 1}},
        {{-0.5, 0.5, -0.5}, {1, 1}},

        //
        {{-0.5, -0.5, -0.5}, {0, 0}},
        {{-0.5, -0.5, 0.5}, {1, 0}},
        {{-0.5, 0.5, -0.5}, {0, 1}},
        {{-0.5, 0.5, 0.5}, {1, 1}},

        //
        {{-0.5, 0.5, 0.5}, {0, 0}},
        {{0.5, 0.5, 0.5}, {1, 0}},
        {{-0.5, 0.5, -0.5}, {0, 1}},
        {{0.5, 0.5, -0.5}, {1, 1}},

        //
        {{-0.5, -0.5, -0.5}, {0, 0}},
        {{0.5, -0.5, -0.5}, {1, 0}},
        {{-0.5, -0.5, 0.5}, {0, 1}},
        {{0.5, -0.5, 0.5}, {1, 1}},
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[VBO_LIGHT]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    u32 indices[kNumIndices] = {
        0,  1,  2,  1,  3,  2,  //
        4,  5,  6,  5,  7,  6,  //
        8,  9,  10, 9,  11, 10, //
        12, 13, 14, 13, 15, 14, //
        16, 17, 18, 17, 19, 18, //
        20, 21, 22, 21, 23, 22, //
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[EBO_LIGHT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Attribute: position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (any)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
}

Camera camera{};
f32 fov = 60.0f;

f32 destPitch = camera.get_pitch();
f32 destYaw = camera.get_yaw();
f32 duration = 0.4;
f32 elapsed = 0;

glm::vec3 lightPosition = {0.25, 0.5, 2};

void render(u32 width, u32 height, const f32 *view_matrix) {
  glViewport(0, 0, width, height);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render the cube

  // Active shader programs before setting uniforms
  program_use(lightingProgram);
  program_set_vec3(lightingProgram, "objectColor", 1, 0.5, 0.3);
  program_set_vec3(lightingProgram, "lightColor", 1, 1, 1);
  program_set_vec3(lightingProgram, "lightPosition", glm::value_ptr(lightPosition));
  program_set_vec3(lightingProgram, "cameraPosition", glm::value_ptr(camera.get_position()));

  program_set_mat4f(lightingProgram, "view", view_matrix);

  glm::mat4 projection(1.0);
  projection = glm::perspective(glm::radians(fov), (f32)width / (f32)height, 0.1f, 100.0f);

  program_set_mat4f(lightingProgram, "projection", glm::value_ptr(projection));

  {
    glm::mat4 model(1.0);
    program_set_mat4f(lightingProgram, "model", glm::value_ptr(model));
  }

  glBindVertexArray(VAOs[VAO_CUBE]);
  glDrawElements(GL_TRIANGLES, kNumIndices, GL_UNSIGNED_INT, nullptr);

  { // Render the lamp
    program_use(lightCubeProgram);
    program_set_mat4f(lightCubeProgram, "view", view_matrix);
    program_set_mat4f(lightCubeProgram, "projection", glm::value_ptr(projection));

    glm::mat4 model(1.0);
    model = glm::translate(model, lightPosition);
    model = glm::scale(model, glm::vec3(0.125f));
    program_set_mat4f(lightCubeProgram, "model", glm::value_ptr(model));

    glBindVertexArray(VAOs[VAO_LIGHT]);
    glDrawElements(GL_TRIANGLES, kNumIndices, GL_UNSIGNED_INT, nullptr);
  }
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
      // printf("[UpdateThread] update frame #%d, delta time %lld microseconds\n", frameIndex,
      //        deltaTime);

      auto tick = deltaTime * 0.001f * 0.001f; // Seconds

      // Do some updates

      {
        // Reset camera's transform
        if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_SPACE)) {
          camera.reset();
          fov = 60.0f;
        } else {
          // Camera rotation

          i32 ix = 0, iy = 0;
          f32 fx = 0, fy = 0;
          input_get_mouse_wheel(context->inputSystemState, &ix, &iy, &fx, &fy);

          f32 deltaPitch = 0, deltaYaw = 0;

          /**
           * Calculate destination rotation;
           * Lerp from current rotation to the destination rotation
           */

          if (ix != 0 || iy != 0) {
            f32 wheel_sensitivity = 196.0f;
            deltaPitch = fy * tick * wheel_sensitivity;
            deltaYaw = -fx * tick * wheel_sensitivity;

            destPitch += deltaPitch;
            destYaw += deltaYaw;
            elapsed = tick;
          } else {
            bool pressed = false;
            if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_UP)) {
              deltaPitch += camera.get_rotation_speed() * tick;
              pressed = true;
            }
            if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_DOWN)) {
              deltaPitch -= camera.get_rotation_speed() * tick;
              pressed = true;
            }
            if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_LEFT)) {
              deltaYaw += camera.get_rotation_speed() * tick;
              pressed = true;
            }
            if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_RIGHT)) {
              deltaYaw -= camera.get_rotation_speed() * tick;
              pressed = true;
            }

            if (pressed) {
              destPitch += deltaPitch;
              destYaw += deltaYaw;
              elapsed = tick;
            }
          }

          // Simply jump to the destination
          // camera.rotate_to(camera.get_pitch() + deltaPitch, camera.get_yaw() + deltaYaw);

          // Or using lerp
          f32 currentPitch = camera.get_pitch();
          f32 currentYaw = camera.get_yaw();

          // Check current and destination values not the duration
          if (fabs(currentPitch - destPitch) > 0.0001f || fabs(currentYaw - destYaw) > 0.0001f) {
            elapsed += tick * 2.0f;
            f32 pitch = lerp(currentPitch, destPitch, ease_out_sine(elapsed));
            f32 yaw = lerp(currentYaw, destYaw, ease_out_sine(elapsed));
            // Or linear
            // f32 pitch = lerp(currentPitch, destPitch, elapsed / duration);
            // f32 yaw = lerp(currentYaw, destYaw, elapsed / duration);
            camera.rotate_to(pitch, yaw);
          }

          // Camera movement
          glm::vec3 offset{};
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_W)) {
            offset += camera.get_movement_speed() * tick * camera.get_front();
          }
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_S)) {
            offset += camera.get_movement_speed() * tick * camera.get_back();
          }
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_A)) {
            offset += camera.get_movement_speed() * tick * camera.get_left();
          }
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_D)) {
            offset += camera.get_movement_speed() * tick * camera.get_right();
          }
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_Q)) {
            offset += camera.get_movement_speed() * tick * camera.get_up();
          }
          if (input_is_key_down(context->inputSystemState, SDL_SCANCODE_E)) {
            offset += camera.get_movement_speed() * tick * camera.get_down();
          }

          camera.move(offset);
        }

        //
      }

      Message outgoing{
          .type = MESSAGE_TYPE_RENDER,
          .u32[0] = frameIndex,
      };

      auto view = camera.get_view_matrix();
      memcpy(outgoing.f32, glm::value_ptr(view), sizeof(f32) * 16);

      context->renderThreadMessageQueue->push(outgoing); // Issue render commands

      if (frameIndex > 0) { // Wait for previous frame to be presented
        // printf("[UpdateThread] frame #%d is waiting for frame #%d to be presented\n", frameIndex,
        //        frameIndex - 1);
        auto &frame = frames[frameIndex - 1];
        std::unique_lock<std::mutex> lock(frame.mutex);
        frame.condition.wait(lock, [&]() { return frame.state == FRAME_STATE_PRESENTED; });
      }

      auto cost =
          std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - startTime).count();
      auto lifespan = (u64)(1 / 60.0f * 1e9); // Nanoseconds
      if (cost < lifespan) { sleep_for((double)(lifespan - cost) / 1e9); }

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
      // printf("[RenderThread] render frame #%d\n", message.u32[0]);
      int w, h;
      SDL_GL_GetDrawableSize(context->window, &w, &h);
      render(w, h, message.f32);
      glFinish();
      // printf("[RenderThread] about to present frame #%d\n", message.u32[0]);
      SDL_GL_SwapWindow(context->window);
      // printf("[RenderThread] frame #%d presented\n", message.u32[0]);
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
  event_register(context.eventSystemState, EVENT_CODE_MOUSE_WHEEL, &context, event_on_scroll);
  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    fprintf(stderr, "error initializing SDL: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  auto window = SDL_CreateWindow(
      "neon", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 360,
      SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
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
      case SDL_MOUSEWHEEL: {
        input_system_process_mouse_wheel(context.inputSystemState, event.wheel.x, event.wheel.y,
                                         event.wheel.preciseX, event.wheel.preciseY);
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
  event_deregister(context.eventSystemState, EVENT_CODE_MOUSE_WHEEL, &context, event_on_scroll);
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

bool event_on_scroll(EventCode eventCode, EventContext eventContext, void *sender, void *listener) {
  return true;
}
