#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lanalyzer.h"
#include "last.h"
#include "lcompiler.h"
#include "lerrs.h"
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
  int fd = open(file, O_RDONLY);
  if (fd < 0) {
    perror("Failed to open file");
    return 1;
  }

  struct stat st;
  if (fstat(fd, &st) < 0) {
    perror("Failed to stat file");
    close(fd);
    return 1;
  }

  size_t fsize = st.st_size;
  char *source = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
  if (source == MAP_FAILED) {
    perror("Failed to mmap file");
    close(fd);
    return 1;
  }

  DEBUG_LOG("file size: %ld\n", fsize);

  llexer_t lexer;
  lparser_t parser;
  lanalyzer_t analyzer;
  lexer_init(&lexer, source, fsize);
  analyzer_init(&analyzer);
  parser_init(&parser, &lexer, &analyzer);
  perr_list_t *errs = parser_parse(&parser);
  fbuffer_t buf = new_fbuf(256);

  if (errs != NULL) {
    for (size_t i = 0; i < errs->length; i++) {
      lparse_err_t *err = errs->items[i];
      parse_err_string(err, &buf);
      ERROR_LOG("%s\n", fbuf_get(&buf));
      fbuf_reset(&buf);
    }

    goto cleanup;
  }

  laerr_list_t *aerrs = &analyzer.errs;
  if (aerrs->length > 0) {
    for (size_t i = 0; i < aerrs->length; i++) {
      lanalyzer_err_t *err = aerrs->items[i];
      analyzer_err_string(err, &buf);
      ERROR_LOG("%s\n", fbuf_get(&buf));
      fbuf_reset(&buf);
    }

    goto cleanup;
  }

  last_t ast = parser.ast;
  for (size_t i = 0; i < ast.globals.length; i++) {
    lvar_stmt *assign = ast.globals.items[i];
    lvar_stmt_string(assign, &buf);
    printf("%s\n", fbuf_get(&buf));
    DEBUG_LOG("Used %ld vs. Cap %ld\n", fbuf_used(&buf), buf.cap);
    fbuf_reset(&buf);
  }

  lcompiler_t compiler;
  compiler_init(&compiler, &ast, "main");
  compiler_compile(&compiler);
  compiler_print(&compiler);
  compiler_write(&compiler, "main.o", "x86_64-pc-linux-gnu");
  compiler_free(&compiler);
cleanup:
  fbuf_free(&buf);
  if (munmap(source, fsize) < 0) {
    perror("Failed to unmap file");
  }
  close(fd);
  analyzer_free(&analyzer);
  parser_free(&parser);

  return 0;
}
