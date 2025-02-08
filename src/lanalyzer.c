#include "lanalyzer.h"
#include "llex.h"
#include "llog.h"
#include "ltypes.h"

lscope_t *new_scope(lscope_t *parent) {
  lscope_t *scope = xmalloc(sizeof(lscope_t));
  scope->parent = parent;
  scope->namend = new_list(8);
  return scope;
};

void scope_add(lscope_t *scope, lvar_stmt *var) {
  CHECK_NULL(scope, );
  CHECK_NULL(var, );

  u32 name = hash_len(var->name.start, var->name.len);
  lnamend_t *namend = new_namend(name, var);
  list_push(&scope->namend, namend);
};

const lvar_stmt *scope_find(lscope_t *scope, u32 name) {
  CHECK_NULL(scope, NULL);

  for (u32 i = 0; i < list_len(&scope->namend); i++) {
    lnamend_t *namend = list_get(&scope->namend, i);
    if (namend->name == name) {
      return namend->variable;
    }
  }

  if (scope->parent) {
    return scope_find(scope->parent, name);
  }

  return NULL;
};

void scope_free(lscope_t *scope) {
  CHECK_NULL(scope, );
  list_free(&scope->namend, namend_free);
  if (scope->parent) {
    scope_free(scope->parent);
  }

  xfree(scope);
};

lanalyzer_err_t *new_sym_undef_err(tok_t sym) {
  lanalyzer_err_t *err = xmalloc(sizeof(lanalyzer_err_t));
  err->kind = AEK_SYM_UNDEFINED;
  err->sym_undef.sym = sym;
  return err;
};
void analyzer_err_free(void *err) {
  CHECK_NULL(err, );
  xfree(err);
};

lnamend_t *new_namend(u32 name, const lvar_stmt *var) {
  lnamend_t *namend = xmalloc(sizeof(lnamend_t));
  namend->name = name;
  namend->variable = var;
  return namend;
};
void namend_free(void *namend) {
  CHECK_NULL(namend, );
  xfree(namend);
};

static lvalue_type_t infer_type(lanalyzer_t *ana, lexpr_t *expr) {
  CHECK_NULL(expr, VT_UNTYPED);

  switch (expr->kind) {
  case LEXPK_LITERAL:
    return expr->literal.vt;
  case LEXPK_BINARY: {
    lvalue_type_t lhs = infer_type(ana, expr->binary.left);
    lvalue_type_t rhs = infer_type(ana, expr->binary.right);
    if (lhs == VT_UNTYPED || rhs == VT_UNTYPED) {
      return VT_UNTYPED;
    }

    if (lhs == VT_FLOAT || rhs == VT_FLOAT) {
      return VT_FLOAT;
    }

    return VT_INT;
  } break;
  case LEXPK_VAR: {
    u32 name = hash_len(expr->variable.name.start, expr->variable.name.len);
    const lvar_stmt *var = scope_find(ana->current, name);
    if (var == NULL) {
      lanalyzer_err_t *err = new_sym_undef_err(expr->variable.name);
      list_push(&ana->errs, err);
      return VT_UNTYPED;
    }

    return var->type;
  } break;
  }

  return VT_UNTYPED;
};

void analyzer_init(lanalyzer_t *analyzer) {
  CHECK_NULL(analyzer, );
  analyzer->errs = new_list(8);
  analyzer->global = new_scope(NULL);
  analyzer->current = analyzer->global;
};

void ana_infer_type(lanalyzer_t *ana, lvar_stmt *var) {
  CHECK_NULL(ana, );
  CHECK_NULL(var, );

  scope_add(ana->current, var);

  if (var->type != VT_UNTYPED) {
    return;
  }

  lvalue_type_t type = infer_type(ana, var->val);
  var->type = type;
  DEBUG_LOG("Inferred type: %d\n", type);
};

void analyzer_free(lanalyzer_t *analyzer) {
  CHECK_NULL(analyzer, );
  list_free(&analyzer->errs, analyzer_err_free);
  scope_free(analyzer->current);
};