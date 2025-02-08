#pragma once

#include "errs.h"
#include "last.h"
#include "llex.h"

typedef struct {
  llexer_t *lexer;
  last_t ast;
  err_list_t errs;
} lparser_t;

void parser_init(lparser_t *parser, llexer_t *lexer);
void parser_free(lparser_t *parser);
err_list_t *parser_parse(lparser_t *parser);