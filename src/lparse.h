#pragma once

#include "errs.h"
#include "lanalyzer.h"
#include "last.h"
#include "llex.h"

typedef struct {
  llexer_t *lexer;
  lanalyzer_t *analyzer;
  last_t ast;
  perr_list_t errs;
} lparser_t;

void parser_init(lparser_t *parser, llexer_t *lexer, lanalyzer_t* analyzer);
void parser_free(lparser_t *parser);
perr_list_t *parser_parse(lparser_t *parser);