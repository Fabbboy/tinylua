#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "last.h"
#include "llex.h"
#include "lparse.h"
#include "ltypes.h"
int main() {
  const char source[] = "local test = 123.123 + 32 * 2";

  llexer_t lexer;
  lparser_t parser;
  lexer_init(&lexer, source, sizeof(source));
  parser_init(&parser, &lexer);
  parser_parse(&parser);
  last_t ast = parser.ast;
  fbuffer_t buf = new_fbuf(256);
  for (size_t i = 0; i < ast.globals.length; i++) {
    lvar_stmt *assign = ast.globals.items[i];
    lvar_stmt_string(assign, &buf);
    printf("%s\n", fbuf_get(&buf));
    fbuf_reset(&buf);
  }
  fbuf_free(&buf);

  parser_free(&parser);
}