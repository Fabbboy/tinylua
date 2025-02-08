#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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

  // Cleanup
  if (munmap(source, fsize) < 0) {
    perror("Failed to unmap file");
  }
  close(fd);
  parser_free(&parser);

  return 0;
}
