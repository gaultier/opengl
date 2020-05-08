#include "bmp.h"

#include <stdio.h>
#include <sys/errno.h>

#include "file.h"
#include "utils.h"

void bmp_load(const char file_path[]) {
    u8 header[54] = {0};
    u32 data_pos = 0, width = 0, height = 0, img_size = 0;

    const usize data_capacity = 20000;
    u8* data = ogl_malloc(data_capacity);
    usize data_len = 0;

    u32 res = file_read(file_path, data, data_capacity, &data_len);
    if (res != 0) exit(res);

    if (data_len < 54) {
        fprintf(stderr, "Incomplete bmp header");
        exit(ENODATA);
    }

    if (data[0] != 'B' || data[1] != 'M') {
        fprintf(stderr, "Missing magic number, not a bmp file");
        exit(EINVAL);
    }

    data_pos = data[0x0a];
    img_size = data[0x22];
    width = data[0x12];
    height = data[0x16];

    // Infer if missing
    if (img_size == 0) img_size = width * height * 3;
    if (data_pos == 0) data_pos = 54;
}
