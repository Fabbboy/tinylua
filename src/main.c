#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "llex.h"
int main() {
  const char source[] = "local test = 123";

  llexer_t lexer;
  lexer_init(&lexer, source, sizeof(source));
  while (true) {
    kind_t k = lexer_next(&lexer);
    printf("Token: %d %.*s\n", k, (int)lexer.curr.len, lexer.curr.start);
    if (k == KIND_EOF)
      break;
  }
}