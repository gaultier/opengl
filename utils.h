#pragma once
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef size_t usize;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef float f32;

#define MIN(a, b) ((a) < (b)) ? (a) : (b)
#define CLAMP(x, xmin, xmax) \
    ((x) < (xmin) ? (xmin) : (x) > (xmax) ? (xmax) : (x))

#define ARR_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static inline void nul_terminate(u8* buffer, usize len) {
    buffer[len - 1] = '\0';
}

static inline void* ogl_malloc(usize size) {
    void* mem = malloc(size);
    if (!mem) exit(ENOMEM);
    return mem;
}

static inline i32 file_read(const char file_path[], u8* content,
                            usize content_capacity, usize* content_len) {
    FILE* file = NULL;
    if ((file = fopen(file_path, "r")) == NULL) {
        fprintf(stderr, "Could not open the file `%s`: errno=%d error=%s\n",
                file_path, errno, strerror(errno));
        return errno;
    }

    int ret = 0;
    if ((ret = fseek(file, 0, SEEK_END)) != 0) {
        fprintf(stderr,
                "Could not move the file cursor to the end of the file `%s`: "
                "errno=%d error=%s\n",
                file_path, errno, strerror(errno));
        return errno;
    }
    const size_t file_size = (size_t)ftell(file);

    if (file_size > content_capacity) {
        fprintf(stderr, "File too big: %zu > %zu", file_size, content_capacity);
        return E2BIG;
    }

    rewind(file);

    const size_t bytes_read = fread(content, 1, file_size, file);
    if (bytes_read != file_size) {
        fprintf(stderr,
                "Could not read whole file: bytes_read=%zu file_size=%zu\n",
                bytes_read, file_size);
        return EIO;
    }
    *content_len = bytes_read;

    fclose(file);

    return 0;
}
