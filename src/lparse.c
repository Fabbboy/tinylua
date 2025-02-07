#include "lparse.h"
#include "last.h"

void parser_init(lparser_t *parser, llexer_t *lexer) {
  parser->lexer = lexer;
  parser->ast = new_ast();
};