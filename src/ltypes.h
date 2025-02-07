#pragma once

#include <stdint.h>
typedef uint32_t u32;

u32 hash(const char* str);
u32 hash_len(const char* str, u32 len);