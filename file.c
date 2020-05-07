#include "file.h"

#include <errno.h>
#include <stdio.h>

#define BUFFER_SIZE 512

i32 file_read(const char file_path[], u8* content, usize context_capacity,
              usize* content_len) {
    FILE* file = fopen(file_path, "ro");
    if (!file) {
        return errno;
    }

    while ((*content_len + BUFFER_SIZE) < context_capacity) {
        usize bytes_read = fread(&content[*content_len], 1, BUFFER_SIZE, file);
        if (errno) goto err;

        *content_len += bytes_read;
    }

    fclose(file);
    return 0;

err:
    if (file) fclose(file);
    return errno;
}
