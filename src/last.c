#include "last.h"
#include "ltypes.h"

last_t new_ast() {
  last_t ast;
  ast.globals = new_list(8);
  return ast;
}

void ast_free(last_t *ast) { list_free(&ast->globals, NULL); }
