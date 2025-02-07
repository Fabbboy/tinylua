#include <stdbool.h>
#include <stddef.h>

#include "llex.h"
#include "lparse.h"
int main() {
  const char source[] = "local test = 123.2";

  llexer_t lexer;
  lparser_t parser;
  lexer_init(&lexer, source, sizeof(source));
  parser_init(&parser, &lexer);
}