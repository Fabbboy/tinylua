#include "lparse.h"
#include "last.h"
#include "llex.h"
#include "llog.h"
#include "ltypes.h"
#include <stdbool.h>
#include <stdio.h>

static bool next(lparser_t *parser, tok_t *result, kind_t expected[],
                 u32 expected_len) {
  kind_t got = lexer_next(parser->lexer);
  *result = parser->lexer->currTok;

  for (u32 i = 0; i < expected_len; i++) {
    if (expected[i] == got) {
      return true;
    }
  }

  char format[256];
  u32 written = snprintf(format, sizeof(format), "Expected one of [");

  for (u32 i = 0; i < expected_len; i++) {
    int len =
        snprintf(format + written, sizeof(format) - written, "%s%s",
                 kind_names[expected[i]], (i == expected_len - 1) ? "" : ", ");
    if (len < 0 || written + len >= sizeof(format)) {
      break;
    }
    written += len;
  }

  int len = snprintf(format + written, sizeof(format) - written,
                     "], but got: %s\n", kind_names[got]);
  if (len < 0 || written + len >= sizeof(format)) {
    format[sizeof(format) - 1] = '\0';
  } else {
    written += len;
  }

  DEBUG_LOG("Wrote a total of: %d bytes in a %lu buffer\n", written,
            sizeof(format));

  ERROR_LOG("%s", format);
  return false;
}

void parser_init(lparser_t *parser, llexer_t *lexer) {
  CHECK_NULL(parser, )
  CHECK_NULL(lexer, )

  parser->lexer = lexer;
  parser->ast = new_ast();
};

void parser_parse(lparser_t *parser) {
  CHECK_NULL(parser, )
  tok_t tok;
  kind_t expected[] = {KIND_EOF, KIND_LOCAL};
  u32 expected_len = sizeof(expected) / sizeof(expected[0]);
  if (next(parser, &tok, expected, expected_len) == false) {
    DEBUG_LOG("ERR\n");
  } else {
    DEBUG_LOG("SUC\n");
  }

  return;
}