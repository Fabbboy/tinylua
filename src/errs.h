#pragma once

#include "llex.h"
#include "ltypes.h"

#define MAX_EXPECTED_KINDS 8

typedef enum {
  PEK_UNEXPECTED_TOKEN,
  PEK_LEXER_ERROR,
} lparse_err_kind_t;

typedef struct {
  lparse_err_kind_t kind;
  union {
    struct {
      tok_t got;
      kind_t expected[MAX_EXPECTED_KINDS];
      u32 expected_len;
    } unexpected;
    struct {
      tok_t tok;
    } lex_err;
  };
} lparse_err_t;

typedef list_t err_list_t;

lparse_err_t *new_unexpected_err(tok_t got, kind_t *expected, u32 expected_len);
lparse_err_t* new_lexer_err(tok_t got);
void parse_err_free(void *err);
void parse_err_string(lparse_err_t *err, fbuffer_t *buf);