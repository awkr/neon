#pragma once

#include <cstdint>

#define DELETE(ptr)                                                                                \
  delete ptr;                                                                                      \
  ptr = nullptr;

typedef void *any;

typedef int32_t i32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;
