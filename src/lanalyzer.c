#include "lanalyzer.h"
#include "llex.h"
#include "llog.h"

static lvalue_type_t infer_type(lexpr_t *expr) {
  CHECK_NULL(expr, VT_UNTYPED);

  switch (expr->kind) {
  case LEXPK_LITERAL:
    return expr->literal.vt;
  case LEXPK_BINARY: {
    lvalue_type_t lhs = infer_type(expr->binary.left);
    lvalue_type_t rhs = infer_type(expr->binary.right);
    if (lhs == VT_FLOAT || rhs == VT_FLOAT) {
      return VT_FLOAT;
    }

    return VT_INT;
  } break;
  }

  return VT_UNTYPED;
};

void analyzer_init(lanalyzer_t *analyzer) {
  CHECK_NULL(analyzer, );
  analyzer->errs = new_list(8);
};

void ana_infer_type(lanalyzer_t *ana, lvar_stmt *var) {
  CHECK_NULL(ana, );
  CHECK_NULL(var, );

  if (var->type != VT_UNTYPED) {
    return;
  }

  lvalue_type_t type = infer_type(var->val);
  var->type = type;
  DEBUG_LOG("Inferred type: %d\n", type);
};

void analyzer_free(lanalyzer_t *analyzer) {
  CHECK_NULL(analyzer, );
  list_free(&analyzer->errs, NULL);
};