#pragma once
#include "utils.h"

i32 file_read(const char file_path[], u8* content, usize context_max_size,
              usize* content_len);
