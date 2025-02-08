#include "llex.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "llog.h"
#include "ltypes.h"

#define KEYWORDS_COUNT 3
#define NEWT(kind) new_token(kind, lexer->ptr, get_len(lexer))

static struct {
  u32 key;
  const char *inter;
  kind_t value;
  lvalue_type_t type;
} keyword_lookup[KEYWORDS_COUNT] = {
    {0, "local", KIND_LOCAL, VT_UNTYPED},
    {0, "int", KIND_TYPE, VT_INT},
    {0, "float", KIND_TYPE, VT_FLOAT},
};

const char *kind_names[] = {
#define X(a) #a,
    KIND_LIST
#undef X
};

const char *vt_names[] = {
#define X(a) #a,
    VT_LIST
#undef X
};

tok_t new_token(kind_t type, char *start, size_t len) {
  tok_t tok;
  tok.type = type;
  tok.start = start;
  tok.len = len;
  return tok;
};

tok_t empty_token() { return (tok_t){0}; };

void token_string(tok_t *tok, fbuffer_t *buffer) {
  fbuf_write(buffer, "Token { type: %s, start: %.*s, len: %d }\n",
             kind_names[tok->type], tok->len, tok->start, tok->len);
};

/***
LEXER
***/

static char get_curr(llexer_t *lexer) {
  CHECK_NULL(lexer, 0);
  if (lexer->pos > lexer->length) {
    return 0;
  }

  return lexer->src[lexer->pos];
}

static char *get_start(llexer_t *lexer) {
  if (lexer->pos > lexer->length) {
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
  lvalue_type_t vt = VT_UNTYPED;
  u32 len = get_len(lexer);
  u32 hash = hash_len(lexer->ptr, len);
  DEBUG_LOG("computed hash %d for %.*s\n", hash, len, lexer->ptr);
  for (int i = 0; i < KEYWORDS_COUNT; i++) {
    if (hash == keyword_lookup[i].key) {
      kind = keyword_lookup[i].value;
      vt = keyword_lookup[i].type;
    }
  }

  lexer->nextTok = NEWT(kind);
  lexer->nextTok.vt = vt;
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

  lexer->nextTok = NEWT(k);

  return k;
}

static void discard_comment(llexer_t *lexer) {
  while (get_curr(lexer) != '\n' && get_curr(lexer) != 0) {
    lexer->pos++;
  }
}

static void lexer_lex(llexer_t *lexer) {
  CHECK_NULL(lexer, );
  if (lexer->pos > lexer->length) {
    DEBUG_LOG("Reached end of input\n");
    lexer->currTok = empty_token();
    return;
  }

  lex_trivia(lexer);
  char c = get_curr(lexer);
  lexer->ptr = get_start(lexer);
  switch (c) {
  case 0:
    lexer->nextTok = empty_token();
    break;
  case 'a' ... 'z':
  case 'A' ... 'Z':
  case '_':
    lex_ident(lexer);
    break;
  case '0' ... '9':
    lex_number(lexer);
    break;
  case '=':
    lexer->pos++;
    lexer->nextTok = NEWT(KIND_ASSIGN);
    break;
  case '+':
    lexer->pos++;
    lexer->nextTok = NEWT(KIND_PLUS);
    break;
  case '-':
    lexer->pos++;
    if (get_curr(lexer) == '-') {
      lexer->pos++;
      discard_comment(lexer);
      lexer_lex(lexer);
      return;
    }
    lexer->nextTok = NEWT(KIND_MINUS);
    break;
  case '*':
    lexer->pos++;
    lexer->nextTok = NEWT(KIND_STAR);
    break;
  case '/':
    lexer->pos++;
    lexer->nextTok = NEWT(KIND_SLASH);
    break;
  case '(':
    lexer->pos++;
    lexer->nextTok = NEWT(KIND_LPAREN);
    break;
  case ')':
    lexer->pos++;
    lexer->nextTok = NEWT(KIND_RPAREN);
    break;
  case ':':
    lexer->pos++;
    lexer->nextTok = NEWT(KIND_COLON);
    break;
  default:
    lexer->pos++;
    lexer->nextTok = new_token(KIND_ERR, lexer->ptr, 1);
    break;
  }
}

void lexer_init(llexer_t *lexer, const char *src, u32 len) {
  CHECK_NULL(lexer, );
  CHECK_NULL(src, );

  lexer->src = src;
  lexer->length = len;
  DEBUG_LOG("length of input source: %d\n", len);
  lexer->pos = 0;
  lexer->ptr = NULL;
  DEBUG_LOG("Hash table:\n");
  for (int i = 0; i < KEYWORDS_COUNT; i++) {
    keyword_lookup[i].key = hash(keyword_lookup[i].inter);
    DEBUG_LOG("- %s: %d\n", keyword_lookup[i].inter, keyword_lookup[i].key);
  }

  lexer_lex(lexer);
  DEBUG_LOG("Setting nextTok\n");
  lexer->currTok = new_token(KIND_EOF, NULL, 0);
};

kind_t lexer_next(llexer_t *lexer) {
  CHECK_NULL(lexer, KIND_ERR);

  lexer->currTok = lexer->nextTok;
  lexer_lex(lexer);
  return lexer->currTok.type;
}

kind_t lexer_peek(llexer_t *lexer) {
  CHECK_NULL(lexer, KIND_ERR);
  return lexer->nextTok.type;
};