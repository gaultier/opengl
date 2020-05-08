#pragma once
#include "utils.h"

void bmp_load(const char file_path[], u8** data, usize data_capacity,
              usize* data_len, usize* width, usize* height, usize* img_size,
              usize* data_pos);

