#pragma once

#include "ltypes.h"
#include <stddef.h>
typedef enum {
  KIND_EOF,
  KIND_ERR,
  KIND_IDENT,
  KIND_NUMBER,
  KIND_DECIMAL,
  KIND_LOCAL,
  KIND_ASSIGN,
} kind_t;

typedef struct {
  kind_t type;
  char *start;
  size_t len;
} tok_t;

tok_t new_token(kind_t type, char *start, size_t len);
tok_t empty_token();

typedef struct {
  const char *src;
  u32 length;
  u32 pos;
  char *ptr;
  tok_t currTok;
  tok_t nextTok;
} llexer_t;

void lexer_init(llexer_t *lexer, const char *src, u32 len);
kind_t lexer_next(llexer_t *lexer);
kind_t lexer_peek(llexer_t* lexer);