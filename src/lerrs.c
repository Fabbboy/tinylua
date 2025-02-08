#include "lerrs.h"
#include "llog.h"
#include <string.h>

lparse_err_t *new_unexpected_err(tok_t got, kind_t *expected,
                                 u32 expected_len) {
  lparse_err_t *err = xmalloc(sizeof(lparse_err_t));
  err->kind = PEK_UNEXPECTED_TOKEN;
  err->unexpected.got = got;
  if (expected_len > MAX_EXPECTED_KINDS) {
    WARN_LOG("Expected list too long\n");
    expected_len = MAX_EXPECTED_KINDS;
  }

  memcpy(err->unexpected.expected, expected, expected_len * sizeof(kind_t));
  err->unexpected.expected_len = expected_len;

  return err;
}

lparse_err_t *new_lexer_err(tok_t got) {
  lparse_err_t *err = xmalloc(sizeof(lparse_err_t));
  err->kind = PEK_LEXER_ERROR;
  err->lex_err.tok = got;
  return err;
};

void parse_err_free(void *err) {
  CHECK_NULL(err, );
  xfree(err);
};

void parse_err_string(lparse_err_t *err, fbuffer_t *buf) {
  CHECK_NULL(err, );
  CHECK_NULL(buf, );

  switch (err->kind) {
  case PEK_UNEXPECTED_TOKEN:
    fbuf_write(buf, "Unexpected token: %s, expected one of [",
               kind_names[err->unexpected.got.type]);
    for (u32 i = 0; i < err->unexpected.expected_len; i++) {
      fbuf_write(buf, "%s%s", kind_names[err->unexpected.expected[i]],
                 (i == err->unexpected.expected_len - 1) ? "" : ", ");
    }
    fbuf_write(buf, "]");
    break;
  case PEK_LEXER_ERROR:
    fbuf_write(buf, "Lexer error: %s", kind_names[err->lex_err.tok.type]);
    break;
  }
};