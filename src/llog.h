#pragma once

#include <stddef.h>

typedef enum {
  DEBUG,
  WARN,
  ERROR,
} llog_level_t;

void llog(llog_level_t level, const char* fmt, ...);
#define CHECK_NULL(X, Y)                                                     \
  if (X == NULL) {                                                           \
    llog(ERROR, "Provided NULL argument to \"%s\" at %s:%d\n", #X, __FILE__, \
         __LINE__);                                                          \
    return Y;                                                                \
  }

#define DEBUG_LOG(...) llog(DEBUG, __VA_ARGS__)
#define WARN_LOG(...) llog(WARN, __VA_ARGS__)
#define ERROR_LOG(...) llog(ERROR, __VA_ARGS__)