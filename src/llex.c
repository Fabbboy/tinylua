#include "llex.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "llog.h"
#include "ltypes.h"

#define KEYWORDS_COUNT 1
#define NEWT(kind) new_token(kind, lexer->ptr, get_len(lexer))

static struct {
  u32 key;
  const char *inter;
  kind_t value;
} keyword_lookup[KEYWORDS_COUNT] = {
    {0, "local", KIND_LOCAL},
};

tok_t new_token(kind_t type, char *start, size_t len) {
  tok_t tok;
  tok.type = type;
  tok.start = start;
  tok.len = len;
  return tok;
};

tok_t empty_token() { return (tok_t){0}; };

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
  DEBUG_LOG("Hash table:\n");
  for (int i = 0; i < KEYWORDS_COUNT; i++) {
    keyword_lookup[i].key = hash(keyword_lookup[i].inter);
    DEBUG_LOG("- %s: %d\n", keyword_lookup[i].inter, keyword_lookup[i].key);
  }
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

  kind_t kind = KIND_IDENT;
  u32 len = get_len(lexer);
  u32 hash = hash_len(lexer->ptr, len);
  DEBUG_LOG("computed hash %d for %.*s\n", hash, len, lexer->ptr);
  for (int i = 0; i < KEYWORDS_COUNT; i++) {
    if (hash == keyword_lookup[i].key) {
      kind = keyword_lookup[i].value;
    }
  }

  lexer->curr = NEWT(kind);
  return kind;
}

static kind_t lex_number(llexer_t *lexer) {
  while (isdigit(get_curr(lexer)))
    lexer->pos++;

  kind_t k = KIND_NUMBER;
  if (get_curr(lexer) == '.') {
    lexer->pos++;
    while (isdigit(get_curr(lexer)))
      lexer->pos++;

    k = KIND_DECIMAL;
  }

  lexer->curr = NEWT(k);

  return k;
}

kind_t lexer_next(llexer_t *lexer) {
  CHECK_NULL(lexer, KIND_ERR);
  if (lexer->pos >= lexer->length) {
    DEBUG_LOG("Reached end of input\n");
    lexer->curr = empty_token();
    return KIND_EOF;
  }

  lex_trivia(lexer);
  char c = get_curr(lexer);
  lexer->ptr = get_start(lexer);
  switch (c) {
  case 0:
    lexer->curr = empty_token();
    return KIND_EOF;
  case 'a' ... 'z':
  case 'A' ... 'Z':
  case '_':
    return lex_ident(lexer);
  case '0' ... '9':
    return lex_number(lexer);
  case '=':
    lexer->pos++;
    lexer->curr = NEWT(KIND_ASSIGN);
    return KIND_ASSIGN;
  default:
    lexer->pos++;
    lexer->curr = new_token(KIND_ERR, lexer->ptr, 1);
    return KIND_ERR;
  }

  return KIND_EOF;
}