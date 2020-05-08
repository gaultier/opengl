#include "bmp.h"

#include <assert.h>
#include <stdio.h>
#include <sys/errno.h>

#include "file.h"

void bmp_load(const char file_path[], u8** data, usize data_capacity,
              usize* data_len, usize* width, usize* height) {
    assert(data != NULL);
    const u8 header_len = 54;
    assert(data_capacity >= header_len);

    u32 data_pos = 0, img_size = 0;

    u32 res = file_read(file_path, *data, data_capacity, data_len);
    if (res != 0) exit(res);

    if (*data_len < header_len) {
        fprintf(stderr, "Incomplete bmp header: %zu < %hhu", *data_len,
                header_len);
        exit(ENODATA);
    }

    if ((*data)[0] != 'B' || (*data)[1] != 'M') {
        fprintf(stderr, "Missing magic number, not a bmp file");
        exit(EINVAL);
    }

    data_pos = *(u32*)&((*data)[0x0a]);
    img_size = *(u32*)&((*data)[0x22]);
    *width = *(u32*)&((*data)[0x12]);
    *height = *(u32*)&((*data)[0x16]);

    // Infer if missing
    if (img_size == 0) img_size = *width * *height * 3;
    if (data_pos == 0) data_pos = header_len;
}
