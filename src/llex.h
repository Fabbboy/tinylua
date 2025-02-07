#pragma once

#include "ltypes.h"
#include <stddef.h>

#define KIND_LIST                                                              \
  X(EOF)                                                                       \
  X(ERR)                                                                       \
  X(IDENT)                                                                     \
  X(NUMBER)                                                                    \
  X(DECIMAL)                                                                   \
  X(LOCAL)                                                                     \
  X(ASSIGN)                                                                    \
  X(PLUS)                                                                      \
  X(MINUS)                                                                     \
  X(STAR)                                                                      \
  X(SLASH)

typedef enum {
#define X(A) KIND_##A,
  KIND_LIST
#undef X
} kind_t;

extern const char *kind_names[];

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
kind_t lexer_peek(llexer_t *lexer);