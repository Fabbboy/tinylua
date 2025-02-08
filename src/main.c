#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "last.h"
#include "llex.h"
#include "llog.h"
#include "lparse.h"
#include "ltypes.h"

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <source>\n", argv[0]);
    return 1;
  }

  const char *file = argv[1];
  FILE *f = fopen(file, "r");
  if (f == NULL) {
    printf("Failed to open file: %s\n", file);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *source = xmalloc(fsize + 1);
  fread(source, 1, fsize, f);
  fclose(f);

  source[fsize] = 0;
  DEBUG_LOG("file size: %ld\n", fsize);

  llexer_t lexer;
  lparser_t parser;
  lexer_init(&lexer, source, fsize);
  parser_init(&parser, &lexer);
  parser_parse(&parser);
  last_t ast = parser.ast;
  fbuffer_t buf = new_fbuf(256);
  for (size_t i = 0; i < ast.globals.length; i++) {
    lvar_stmt *assign = ast.globals.items[i];
    lvar_stmt_string(assign, &buf);
    printf("%s\n", fbuf_get(&buf));
    DEBUG_LOG("Used %ld vs. Cap %ld\n", fbuf_used(&buf), buf.cap);
    fbuf_reset(&buf);
  }
  fbuf_free(&buf);

  xfree(source);
  parser_free(&parser);
}