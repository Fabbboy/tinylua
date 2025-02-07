#pragma once

#include "last.h"
#include "llex.h"

typedef struct {
  llexer_t *lexer;
  last_t ast;
} lparser_t;

void parser_init(lparser_t *parser, llexer_t *lexer);