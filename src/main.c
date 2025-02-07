#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "last.h"
#include "llex.h"
#include "lparse.h"
int main() {
  const char source[] = "local test = 123.123";

  llexer_t lexer;
  lparser_t parser;
  lexer_init(&lexer, source, sizeof(source));
  parser_init(&parser, &lexer);
  parser_parse(&parser);
  last_t ast = parser.ast;
  for (size_t i = 0; i < ast.globals.length; i++) {
    lvar_stmt *assign = ast.globals.items[i];
    printf("Name: %.*s, Value: %f\n", (u32)assign->name.len, assign->name.start,
           assign->val->literal.fval);
  }

  parser_free(&parser);
}