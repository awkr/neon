#pragma once

#include "defines.h"
#include <mutex>
#include <queue>

enum MessageType {
  MESSAGE_TYPE_NONE = 0x0,
  MESSAGE_TYPE_QUIT,
  MESSAGE_TYPE_UPDATE,
  MESSAGE_TYPE_RESIZED,
  MESSAGE_TYPE_RENDER,
  MESSAGE_TYPE_PRESENT,
};

enum MessagePriority {
  MESSAGE_PRIORITY_LOW = 0x0,
  MESSAGE_PRIORITY_NORMAL,
  MESSAGE_PRIORITY_HIGH,
};

struct Message {
  MessageType type = MESSAGE_TYPE_NONE;
  MessagePriority priority = MESSAGE_PRIORITY_NORMAL;
  u64 u64 = 0;
  u32 u32[2] = {};
  f32 f32[16];

  bool operator<(const Message &rhs) const { return priority < rhs.priority; }
};

class MessageQueue {
public:
  explicit MessageQueue(u32 capacity = 65536) : _capacity{capacity} {}

  ~MessageQueue() {
    std::unique_lock<std::mutex> lock(_mutex);
    _closeFlag = true;
  }

  bool push(Message message) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_closeFlag) { return false; }
    // if (_queue.size() >= _capacity) { return false; }
    _queue.push(message);
    _condition.notify_one(); // Notify one thread that is waiting
    return true;
  }

  bool pop(Message *message) {
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this]() { return !_closeFlag && !_queue.empty(); });
    if (_closeFlag) { return false; }
    *message = _queue.top();
    _queue.pop();
    return true;
  }

private:
  std::priority_queue<Message> _queue;
  u32 _capacity;
  std::mutex _mutex;
  std::condition_variable _condition;
  bool _closeFlag;
};
