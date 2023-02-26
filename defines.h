#pragma once

#include <cmath>
#include <cstdint>
#include <cstdlib>

#define PI 3.14159265358979f

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

inline f32 lerp(f32 a, f32 b, f32 p) { return a + p * (b - a); }

inline f32 ease_in_out_sine(f32 p) { return -(cos(PI * p) - 1) * 0.5f; }

inline f32 ease_in_out_expo(f32 p) {
  if (p < 0.5) {
    return (pow(2, 16 * p) - 1) / 510.0;
  } else {
    return 1 - 0.5 * pow(2, -16 * (p - 0.5));
  }
}

inline f32 ease_out_sine(f32 p) { return sin(PI * p * 0.5); }
