#include "llog.h"

#include <stdarg.h>
#include <stdio.h>

static inline const char* ansi_code(llog_level_t lvl) {
  switch (lvl) {
    case DEBUG:
      return "\e[38;5;248m";
    case WARN:
      return "\033[1;33m";
    case ERROR:
      return "\033[1;31m";
  }
  return "";
}

void llog(llog_level_t level, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  fprintf(stderr, "%s", ansi_code(level));
  vfprintf(stderr, fmt, args);  // this crashes
  fprintf(stderr, "\033[0m");

  va_end(args);
}