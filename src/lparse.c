#include "lparse.h"
#include "last.h"
#include "llex.h"
#include "llog.h" 

void parser_init(lparser_t *parser, llexer_t *lexer) {
  CHECK_NULL(parser, )
  CHECK_NULL(lexer, )

  parser->lexer = lexer;
  parser->ast = new_ast();
};

void parser_parse(lparser_t *parser) {
  CHECK_NULL(parser, )

  return;
}