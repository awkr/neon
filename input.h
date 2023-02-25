#pragma once

#include "defines.h"

enum KeyCode {
  KEY_CODE_NONE = 0x0,

  KEY_CODE_ESC,
  KEY_CODE_SPACE,
  KEY_CODE_TAB,
  KEY_CODE_LEFT_SHIFT,
  KEY_CODE_CTRL,
  KEY_CODE_DELETE,
  KEY_CODE_A,
  KEY_CODE_C,
  KEY_CODE_D,
  KEY_CODE_E,
  KEY_CODE_Q,
  KEY_CODE_S,
  KEY_CODE_W,
  KEY_CODE_Z,
  KEY_CODE_UP,
  KEY_CODE_LEFT,
  KEY_CODE_DOWN,
  KEY_CODE_RIGHT,

  KEY_CODE_COUNT,
};

void input_system_initialize(void **state, void *eventSystemState);
void input_system_shutdown(void **state);
void input_system_update(void *state);
bool input_system_process_key(void *state, u16 key, bool pressed);
bool input_system_process_mouse_wheel(void *state, f32 x, f32 y);
bool input_is_key_down(void *state, u16 key);
bool input_was_key_down(void *state, u16 key);
