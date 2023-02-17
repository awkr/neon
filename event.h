#pragma once

#include "defines.h"

enum EventCode {
  EVENT_CODE_NONE = 0x00,

  EVENT_CODE_QUIT = 0x01,
  EVENT_CODE_RESIZED,
  EVENT_CODE_KEYBOARD_PRESSED,
  EVENT_CODE_KEYBOARD_RELEASED,
  EVENT_CODE_BUTTON_PRESSED,
  EVENT_CODE_BUTTON_RELEASED,
  EVENT_CODE_MOUSE_MOVED,
  EVENT_CODE_MOUSE_WHEEL,

  EVENT_CODE_DEBUG_0,
  EVENT_CODE_DEBUG_1,
  EVENT_CODE_DEBUG_2,
  EVENT_CODE_DEBUG_3,
  EVENT_CODE_DEBUG_4,

  EVENT_CODE_COUNT,
};

struct EventContext {
  union { // 128 bytes
    i64 i64[2];
    u64 u64[2];
    f64 f64[2];

    i32 i32[4];
    u32 u32[4];
    f32 f32[4];

    i16 i16[8];
    u16 u16[8];

    i8 i8[16];
    u8 u8[16];

    char c[16];
  };
};

typedef bool (*PFN_on_event)(EventContext context, void *sender);

void event_system_initialize(void **state);
void event_system_shutdown(void **state);

bool event_register(void *state, EventCode code, void *listener, PFN_on_event fn);
bool event_deregister(void *state, EventCode code, const void *listener, PFN_on_event fn);
bool event_fire(void *state, EventCode code, void *sender, EventContext context);
