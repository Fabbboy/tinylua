#include "ltypes.h"

u32 hash(const char *str) {
  u32 base = 5381;
  u32 c;
  while ((c = *str++)) {
    base = ((base << 5) + base) + c;
  }

  return base;
};

u32 hash_len(const char *str, u32 len) {
  u32 base = 5381;
  u32 c;
  for (u32 i = 0; i < len; i++) {
    c = str[i];
    base = ((base << 5) + base) + c;
  }

  return base;
};