#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "last.h"
#include "llex.h"
#include "lparse.h"
int main() {
  const char source[] = "local test = 123.123 + 32 * 2";

  llexer_t lexer;
  lparser_t parser;
  lexer_init(&lexer, source, sizeof(source));
  parser_init(&parser, &lexer);
  parser_parse(&parser);
  last_t ast = parser.ast;
  for (size_t i = 0; i < ast.globals.length; i++) {
    lvar_stmt *assign = ast.globals.items[i];
    lexpr_t *val = assign->val;
    lexpr_t *left = val->binary.left;           // is a literal
    lexpr_t *right = val->binary.right;         // is a binary
    lexpr_t *right_left = right->binary.left;   // is a literal
    lexpr_t *right_right = right->binary.right; // is a literal

    printf("local %.*s = %f + %ld * %ld\n", (int)assign->name.len,
           assign->name.start, left->literal.fval, right_left->literal.ival,
           right_right->literal.ival);
  }

  parser_free(&parser);
}