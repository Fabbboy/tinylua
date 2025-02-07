#include <stdbool.h>
#include <stddef.h>

#include "llex.h"
#include "lparse.h"
int main() {
  const char source[] = "asd local test = 123.2";

  llexer_t lexer;
  lparser_t parser;
  lexer_init(&lexer, source, sizeof(source));
  parser_init(&parser, &lexer);
  parser_parse(&parser);

  /*   while (true) {
      kind_t k = lexer_next(&lexer);
      tok_t t = lexer.currTok;
      printf("Tok: kind: %d, val: %.*s\n", k, (u32)t.len, t.start);
      if (k == KIND_EOF)
        break;
    } */
}