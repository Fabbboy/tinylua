#pragma once

#include "ltypes.h"
#include <stddef.h>
typedef enum {
  KIND_EOF,
  KIND_ERR,
  KIND_IDENT,
  KIND_NUMBER,
  KIND_LOCAL,
} kind_t;

typedef struct {
  kind_t type;
  char *start;
  size_t len;
} tok_t;

tok_t new_token(kind_t type, char *start, size_t len);

typedef struct {
  const char *src;
  u32 length;
  u32 pos;
  char *ptr;
  tok_t curr;
  tok_t next;
} llexer_t;

void lexer_init(llexer_t *lexer, const char *src, u32 len);
kind_t lexer_next(llexer_t *lexer);