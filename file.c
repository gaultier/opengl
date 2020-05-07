#include "file.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include "malloc.h"

#define BUFFER_SIZE 512

i32 file_read(const char file_path[], u8* content, usize content_capacity,
              usize* content_len) {
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
    nul_terminate(content, *content_len);

    fclose(file);
}
