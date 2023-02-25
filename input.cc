#include "input.h"
#include "event.h"
#include <cstdio>
#include <cstring>

static const u16 KEY_COUNT = 512;

struct KeyboardState {
  bool keys[KEY_COUNT];
};

struct InputSystemState {
  KeyboardState keyboardCurrent;
  KeyboardState keyboardPrevious;
  void *eventSystemState;
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

bool input_system_process_mouse_wheel(void *state, f32 x, f32 y) {
  auto s = (InputSystemState *)state;
  EventContext context{};
  context.f32[0] = x;
  context.f32[1] = y;
  event_fire(s->eventSystemState, EVENT_CODE_MOUSE_WHEEL, nullptr, context);
  return true;
}

bool input_is_key_down(void *state, u16 key) {
  auto s = (InputSystemState *)state;
  return s->keyboardCurrent.keys[key];
}

bool input_was_key_down(void *state, u16 key) {
  auto s = (InputSystemState *)state;
  return s->keyboardPrevious.keys[key];
}
