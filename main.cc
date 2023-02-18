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

#include <OpenGL/gl3.h>
#include <SDL.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
// #include <tinycthread.h>

// typedef struct {
//   GLFWwindow *window;
//   const char *title;
//   float r, g, b;
//   thrd_t id;
// } Thread;
//
// static volatile int running = GLFW_TRUE;
//
// static void error_callback(int error, const char *description) {
//   fprintf(stderr, "Error: %s\n", description);
// }
//
// static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
//   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window,
//   GLFW_TRUE);
// }
//
// static int thread_main(void *data) {
//   const Thread *thread = (Thread *)data;
//
//   glfwMakeContextCurrent(thread->window);
//   glfwSwapInterval(1);
//
//   while (running) {
//     // const float v = (float)fabs(sin(glfwGetTime() * 2.f));
//     float v = 1;
//     glClearColor(thread->r * v, thread->g * v, thread->b * v, 0.f);
//
//     glClear(GL_COLOR_BUFFER_BIT);
//     glfwSwapBuffers(thread->window);
//   }
//
//   glfwMakeContextCurrent(NULL);
//   return 0;
// }

int main(int argc, char **argv) {
  bool quit = false;
  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    fprintf(stderr, "error initializing SDL: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
  auto window = SDL_CreateWindow(
      "neon", 0, 0, 640, 480, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
  auto glContext = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, glContext);
  SDL_Event event;
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: quit = true; break;
      case SDL_KEYUP:
        switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_ESCAPE: quit = true; break;
        default: break;
        }
        break;
      default: break;
      }
    }
    if (quit) { break; }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    SDL_GL_SwapWindow(window);
  }
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;

  // int i, result;
  // Thread threads[] = {{NULL, "Red", 1.f, 0.f, 0.f, 0}};
  // const int count = sizeof(threads) / sizeof(Thread);
  //
  // glfwSetErrorCallback(error_callback);
  //
  // if (!glfwInit()) exit(EXIT_FAILURE);
  //
  // for (i = 0; i < count; i++) {
  //   glfwWindowHint(GLFW_POSITION_X, 200 + 250 * i);
  //   glfwWindowHint(GLFW_POSITION_Y, 200);
  //
  //   threads[i].window = glfwCreateWindow(200, 200, threads[i].title, NULL, NULL);
  //   if (!threads[i].window) {
  //     glfwTerminate();
  //     exit(EXIT_FAILURE);
  //   }
  //
  //   glfwSetKeyCallback(threads[i].window, key_callback);
  // }
  //
  // // glfwMakeContextCurrent(threads[0].window);
  // // glfwMakeContextCurrent(NULL);
  //
  // for (i = 0; i < count; i++) {
  //   if (thrd_create(&threads[i].id, thread_main, threads + i) != thrd_success) {
  //     fprintf(stderr, "Failed to create secondary thread\n");
  //
  //     glfwTerminate();
  //     exit(EXIT_FAILURE);
  //   }
  // }
  //
  // while (running) {
  //   glfwWaitEvents();
  //
  //   for (i = 0; i < count; i++) {
  //     if (glfwWindowShouldClose(threads[i].window)) running = GLFW_FALSE;
  //   }
  // }
  //
  // for (i = 0; i < count; i++)
  //   glfwHideWindow(threads[i].window);
  //
  // for (i = 0; i < count; i++)
  //   thrd_join(threads[i].id, &result);
  //
  // exit(EXIT_SUCCESS);
}
