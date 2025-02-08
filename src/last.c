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

  switch (expr->kind) {
  case LEXPK_LITERAL:
    break;
  case LEXPK_BINARY:
    lexpr_free(expr->binary.left);
    lexpr_free(expr->binary.right);
    break;
  }

  xfree(expr);
};

void lexpr_string(lexpr_t *expr, fbuffer_t *buf) {
  CHECK_NULL(expr, );
  CHECK_NULL(buf, );

  switch (expr->kind) {
  case LEXPK_LITERAL:
    fbuf_write(buf, "Literal { ");
    switch (expr->literal.vt) {
    case VT_FLOAT:
      fbuf_write(buf, "type: float, value: %f }", expr->literal.fval);
      break;
    case VT_INT:
      fbuf_write(buf, "type: int, value: %ld }", expr->literal.ival);
      break;
    case VT_UNTYPED:
      fbuf_write(buf, "type: untyped }");
      break;
    }
    break;
  case LEXPK_BINARY:
    fbuf_write(buf, "Binary { op: %d, left: ", expr->binary.op);
    lexpr_string(expr->binary.left, buf);
    fbuf_write(buf, ", right: ");
    lexpr_string(expr->binary.right, buf);
    fbuf_write(buf, " }");
    break;
  }
};

void lvar_stmt_string(lvar_stmt *stmt, fbuffer_t *buf) {
  CHECK_NULL(stmt, );
  CHECK_NULL(buf, );

  fbuf_write(buf, "Variable { name: %.*s, linkage: %d, type: %d value: ",
             stmt->name.len, stmt->name.start, stmt->link, stmt->type);
  lexpr_string(stmt->val, buf);
  fbuf_write(buf, " }");
};

lvar_stmt *new_lvar_stmt(tok_t name, lexpr_t *val, linkage_t link,
                         value_type_t type) {
  lvar_stmt *stmt = xmalloc(sizeof(lvar_stmt));
  stmt->name = name;
  stmt->val = val;
  stmt->link = link;
  stmt->type = type;
  return stmt;
};

void lvar_stmt_free(void *item) {
  CHECK_NULL(item, );

  lvar_stmt *stmt = (lvar_stmt *)item;
  lexpr_free(stmt->val);
  xfree(stmt);
};
