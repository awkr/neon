#pragma once

#include <cstdint>
#include <cstdlib>

#define DELETE(ptr)                                                                                \
  delete ptr;                                                                                      \
  ptr = nullptr;

static const size_t KiB = 1024u;
static const size_t MiB = 1024u * KiB;

typedef void *any;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;
