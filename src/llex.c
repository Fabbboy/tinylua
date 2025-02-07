#include "llex.h"

#include <ctype.h>
#include <stdbool.h>

#include "llog.h"

tok_t new_token(kind_t type) {
  tok_t tok;
  tok.type = type;
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
  llog(DEBUG, "length of input source: %d\n", len);
};

static char getCurr(llexer_t *lexer) {
  CHECK_NULL(lexer, 0);
  if (lexer->pos >= lexer->length) {
    return 0;
  }

  return lexer->src[lexer->pos];
}

static char *getStart(llexer_t *lexer) {
  if (lexer->pos >= lexer->length) {
    return NULL;
  }

  return (char *)lexer->src + lexer->pos;
}

static void lexTrivia(llexer_t *lexer) {
  char c = getCurr(lexer);
  while (true) {
    if (!isspace(c))
      break;
    lexer->pos++;
    c = getCurr(lexer);
  }
}

kind_t lexer_next(llexer_t *lexer) {
  CHECK_NULL(lexer, KIND_ERR);
  if (lexer->pos >= lexer->length) {
    DEBUG_LOG("Reached end of input");
    return KIND_EOF;
  }

  lexTrivia(lexer);
  char c = getCurr(lexer);
  lexer->ptr = getStart(lexer);
  switch (c) {
  case 0:
    return KIND_EOF;
  default:
    lexer->pos++;
    lexer->curr = new_token(KIND_ERR);
    return KIND_ERR;
  }

  return KIND_EOF;
}