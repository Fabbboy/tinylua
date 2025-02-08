#include "lanalyzer.h"
#include "leval.h"
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

lanalyzer_err_t *new_internal_err(char *msg) {
  lanalyzer_err_t *err = xmalloc(sizeof(lanalyzer_err_t));
  err->kind = AEK_INTERNAL;
  err->internal.msg = msg;
  return err;
};

void analyzer_err_free(void *err) {
  CHECK_NULL(err, );
  lanalyzer_err_t *err_ = (lanalyzer_err_t *)err;
  if (err_->kind == AEK_INTERNAL) {
    xfree(err_->internal.msg);
  }

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

lanalyzer_err_t *new_type_mismatch_err(lvalue_type_t expected,
                                       lvalue_type_t got) {
  lanalyzer_err_t *err = xmalloc(sizeof(lanalyzer_err_t));
  err->kind = AEK_TYPE_MISMATCH;
  err->type_mismatch.expected = expected;
  err->type_mismatch.got = got;
  return err;
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

lanalyzer_err_t *new_div_by_zero_err() {
  lanalyzer_err_t *err = xmalloc(sizeof(lanalyzer_err_t));
  err->kind = AEK_DIV_BY_ZERO;
  return err;
};

void analyzer_init(lanalyzer_t *analyzer) {
  CHECK_NULL(analyzer, );
  analyzer->errs = new_list(8);
  analyzer->global = new_scope(NULL);
  analyzer->current = analyzer->global;
};

void ana_infer_type(lanalyzer_t *ana, lvar_stmt *var, bool eval) {
  CHECK_NULL(ana, );
  CHECK_NULL(var, );

  scope_add(ana->current, var);
  lvalue_type_t type = infer_type(ana, var->val);
  if (type == VT_UNTYPED) {
    return;
  }

  if (var->type == VT_UNTYPED) {
    var->type = type;
  }

  if (var->type != type) {
    lanalyzer_err_t *err = new_type_mismatch_err(var->type, type);
    list_push(&ana->errs, err);
  }

  if (eval) {
    lanalyzer_err_t *err = eval_var(ana, var);
    if (err != NULL) {
      list_push(&ana->errs, err);
    }
  }
};

void analyzer_err_string(lanalyzer_err_t *err, fbuffer_t *buf) {
  CHECK_NULL(err, );
  CHECK_NULL(buf, );

  switch (err->kind) {
  case AEK_SYM_UNDEFINED:
    fbuf_write(buf, "Symbol '%.*s' is undefined", err->sym_undef.sym.len,
               err->sym_undef.sym.start);
    break;
  case AEK_TYPE_MISMATCH:
    fbuf_write(buf, "Type mismatch: expected %s, got %s",
               vt_names[err->type_mismatch.expected],
               vt_names[err->type_mismatch.got]);
    break;
  case AEK_INTERNAL:
    fbuf_write(buf, "Internal error: %s", err->internal.msg);
    break;
  case AEK_DIV_BY_ZERO:
    fbuf_write(buf, "Division by zero");
    break;
  }
};

void analyzer_free(lanalyzer_t *analyzer) {
  CHECK_NULL(analyzer, );
  list_free(&analyzer->errs, analyzer_err_free);
  scope_free(analyzer->current);
};