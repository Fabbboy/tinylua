#include "llex.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

#include "llog.h"

tok_t new_token(kind_t type, char *start, size_t len) {
  tok_t tok;
  tok.type = type;
  tok.start = start;
  tok.len = len;
  return tok;
};

/***
LEXER
***/

void lexer_init(llexer_t *lexer, const char *src, u32 len) {
  CHECK_NULL(lexer, );
  CHECK_NULL(src, );

  lexer->src = src;
  lexer->length = len;
  lexer->pos = 0;
  lexer->ptr = NULL;
  lexer->curr = new_token(KIND_EOF, NULL, 0);
  lexer->next = new_token(KIND_EOF, NULL, 0);
  DEBUG_LOG("length of input source: %d\n", len);
};

static char get_curr(llexer_t *lexer) {
  CHECK_NULL(lexer, 0);
  if (lexer->pos >= lexer->length) {
    return 0;
  }

  return lexer->src[lexer->pos];
}

static char *get_start(llexer_t *lexer) {
  if (lexer->pos >= lexer->length) {
    return NULL;
  }

  return (char *)lexer->src + lexer->pos;
}

static void lex_trivia(llexer_t *lexer) {
  char c = get_curr(lexer);
  while (isspace(c)) {
    lexer->pos++;
    c = get_curr(lexer);
  }
}

static size_t get_len(llexer_t *lexer) {
  return lexer->pos - (lexer->ptr - lexer->src);
}

static kind_t lex_ident(llexer_t *lexer) {
  while (isalnum(get_curr(lexer))) {
    lexer->pos++;
  }

  lexer->curr = new_token(KIND_IDENT, lexer->ptr, get_len(lexer));
  return KIND_IDENT;
}
kind_t lexer_next(llexer_t *lexer) {
  CHECK_NULL(lexer, KIND_ERR);
  if (lexer->pos >= lexer->length) {
    DEBUG_LOG("Reached end of input\n");
    return KIND_EOF;
  }

  lex_trivia(lexer);
  char c = get_curr(lexer);
  lexer->ptr = get_start(lexer);
  switch (c) {
  case 0:
    return KIND_EOF;
  case 'a' ... 'z':
  case 'A' ... 'Z':
  case '_':
    return lex_ident(lexer);
  default:
    lexer->pos++;
    lexer->curr = new_token(KIND_ERR, lexer->ptr, 1);
    return KIND_ERR;
  }

  return KIND_EOF;
}