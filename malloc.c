#include "malloc.h"

#include <stdlib.h>
#include <sys/errno.h>

void* ogl_malloc(usize size) {
    void* mem = malloc(size);
    if (!mem) exit(ENOMEM);
    return mem;
}
