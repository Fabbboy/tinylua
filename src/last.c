#include "last.h"
#include "llog.h"
#include "ltypes.h"

last_t new_ast() {
  last_t ast;
  ast.globals = new_list(8);
  return ast;
}

void ast_free(last_t *ast) { list_free(&ast->globals, lvar_stmt_free); };

lexpr_t *new_lexpr(lexpr_kind_t kind) {
  lexpr_t *expr = xmalloc(sizeof(lexpr_t));
  expr->kind = kind;
  return expr;
};
void lexpr_free(lexpr_t *expr) {
  CHECK_NULL(expr, );
  xfree(expr);
};

lvar_stmt *new_lvar_stmt(tok_t name, lexpr_t *val) {
  lvar_stmt *stmt = xmalloc(sizeof(lvar_stmt));
  stmt->name = name;
  stmt->val = val;
  return stmt;
};

void lvar_stmt_free(void *item) {
  CHECK_NULL(item, );

  lvar_stmt *stmt = (lvar_stmt *)item;
  xfree(stmt->val);
  xfree(stmt);
};
