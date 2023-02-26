#include "input.h"
#include "event.h"
#include <cstdio>
#include <cstring>
#include <mutex>
#include <queue>

static const u16 KEY_COUNT = 512;

struct KeyboardState {
  bool keys[KEY_COUNT];
};

struct Wheeling {
  i32 ix;
  i32 iy;
  f32 fx;
  f32 fy;
};

struct InputSystemState {
  KeyboardState keyboardCurrent;
  KeyboardState keyboardPrevious;
  void *eventSystemState;
  std::queue<Wheeling> wheeling;
  std::mutex wheeling_mutex;
};

void input_system_initialize(void **state, void *eventSystemState) {
  auto s = new InputSystemState();
  s->eventSystemState = eventSystemState;
  *state = s;
}

void input_system_shutdown(void **state) {
  auto s = (InputSystemState *)*state;
  DELETE(s);
}

void input_system_update(void *state) {
  auto s = (InputSystemState *)state;

  // Handle holding keys
  for (u16 i = 0; i < KEY_COUNT; ++i) {
    if (input_was_key_down(state, i) && input_is_key_down(state, i)) {
      EventContext context{};
      context.u16[0] = i;
      event_fire(s->eventSystemState, EVENT_CODE_KEYBOARD_PRESSED, nullptr, context);
    }
  }

  // Copy current states to previous states
  memcpy(&s->keyboardPrevious, &s->keyboardCurrent, sizeof(KeyboardState));
}

bool input_system_process_key(void *state, u16 key, bool pressed) {
  auto s = (InputSystemState *)state;
  if (s->keyboardCurrent.keys[key] != pressed) {
    s->keyboardCurrent.keys[key] = pressed;
    EventContext context{};
    context.u16[0] = key;
    event_fire(s->eventSystemState,
               pressed ? EVENT_CODE_KEYBOARD_PRESSED : EVENT_CODE_KEYBOARD_RELEASED, nullptr,
               context);
    return true;
  }
  return false;
}

bool input_system_process_mouse_wheel(void *state, i32 ix, i32 iy, f32 fx, f32 fy) {
  auto s = (InputSystemState *)state;
  std::lock_guard<std::mutex> lock(s->wheeling_mutex);
  if (s->wheeling.size() > 2000) { return false; }
  // printf("mouse wheeling %d %d %f %f\n", ix, iy, fx, fy);
  Wheeling wheeling{};
  wheeling.ix = ix;
  wheeling.iy = iy;
  wheeling.fx = fx;
  wheeling.fy = fy;
  s->wheeling.emplace(wheeling);
  return true;
}

bool input_is_key_down(void *state, u16 key) {
  auto s = (InputSystemState *)state;
  return s->keyboardCurrent.keys[key];
}

bool input_is_key_up(void *state, u16 key) { return !input_is_key_down(state, key); }

bool input_was_key_down(void *state, u16 key) {
  auto s = (InputSystemState *)state;
  return s->keyboardPrevious.keys[key];
}

bool input_get_mouse_wheel(void *state, i32 *ix, i32 *iy, f32 *fx, f32 *fy) {
  auto s = (InputSystemState *)state;
  std::lock_guard<std::mutex> lock(s->wheeling_mutex);
  if (s->wheeling.empty()) { return false; }
  auto &front = s->wheeling.front();
  if (ix) { *ix = front.ix; }
  if (iy) { *iy = front.iy; }
  if (fx) { *fx = front.fx; }
  if (fy) { *fy = front.fy; }
  s->wheeling.pop();
  return true;
}
