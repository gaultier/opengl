#include "malloc.h"

#include <stdlib.h>
#include <sys/errno.h>

void* ogl_malloc(usize size) {
    void* mem = malloc(size);
    if (!mem) exit(ENOMEM);
    return mem;
}

void nul_terminate(u8* buffer, usize len) { buffer[len - 1] = '\0'; }
