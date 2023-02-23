#include "event.h"
#include <vector>

struct RegisteredEvent {
  void *listener;
  PFN_on_event fn;
};

struct EventCodeEntry {
  std::vector<RegisteredEvent> events;
};

struct EventSystemState {
  EventCodeEntry entries[EVENT_CODE_COUNT]; // Lookup table for event codes
};

void event_system_initialize(void **state) { *state = new EventSystemState(); }

void event_system_shutdown(void **state) {
  auto s = (EventSystemState *)*state;
  for (auto &entry : s->entries) {
    entry.events.clear();
  }
  DELETE(s)
}

bool event_register(void *state, EventCode code, void *listener, PFN_on_event fn) {
  auto s = (EventSystemState *)state;
  auto &entry = s->entries[code];
  for (const auto &event : entry.events) {
    if (event.listener == listener && event.fn == fn) {
      fprintf(stderr,
              "[error] event has already been registered with the code %u and the callback %p\n",
              code, fn);
      return false;
    }
  }
  RegisteredEvent event{};
  event.listener = listener;
  event.fn = fn;
  entry.events.emplace_back(event);
  return true;
}

bool event_deregister(void *state, EventCode code, const void *listener, PFN_on_event fn) {
  auto s = (EventSystemState *)state;
  auto &entry = s->entries[code];
  for (auto it = entry.events.begin(); it != entry.events.end(); ++it) {
    if ((*it).listener == listener && (*it).fn == fn) {
      entry.events.erase(it);
      return true;
    }
  }
  return false;
}

bool event_fire(void *state, EventCode code, void *sender, EventContext context) {
  auto s = (EventSystemState *)state;
  auto &entry = s->entries[code];
  for (const auto &event : entry.events) {
    if (event.fn(code, context, sender, event.listener)) {
      return true; // Event has been handled, do not send to other listeners
    }
  }
  return false;
}
