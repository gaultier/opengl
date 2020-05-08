#pragma once
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

typedef size_t usize;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

#define MIN(a, b) ((a) < (b)) ? (a) : (b)

static void nul_terminate(u8* buffer, usize len) { buffer[len - 1] = '\0'; }

static void* ogl_malloc(usize size) {
    void* mem = malloc(size);
    if (!mem) exit(ENOMEM);
    return mem;
}

static float degree_to_radian(float degree) { return degree * M_PI / 180; }
