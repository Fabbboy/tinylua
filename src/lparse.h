#pragma once

#include "last.h"
#include "llex.h"
#include "ltypes.h"

typedef struct {
  llexer_t *lexer;
  last_t ast;
  fbuffer_t errBuf;
} lparser_t;

void parser_init(lparser_t *parser, llexer_t *lexer);
void parser_free(lparser_t *parser);
void parser_parse(lparser_t *parser);