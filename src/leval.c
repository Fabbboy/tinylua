#include "leval.h"
#include "lanalyzer.h"
#include "last.h"
#include "llex.h"
#include "llog.h"
#include "ltypes.h"
#include <string.h>

static lanalyzer_err_t *eval_expr(lanalyzer_t *ana, lexpr_t *expr,
                                  lexpr_t *target);

static lanalyzer_err_t *eval_binary(lanalyzer_t *ana, lexpr_t *expr,
                                    lexpr_t *target) {
  CHECK_NULL(ana, NULL);
  CHECK_NULL(expr, NULL);

  lexpr_t left_result = {0};
  lexpr_t right_result = {0};

  lanalyzer_err_t *lhs_err = eval_expr(ana, expr->binary.left, &left_result);
  if (lhs_err != NULL) {
    return lhs_err;
  }

  lanalyzer_err_t *rhs_err = eval_expr(ana, expr->binary.right, &right_result);
  if (rhs_err != NULL) {
    return rhs_err;
  }

  if (left_result.literal.vt == VT_FLOAT ||
      right_result.literal.vt == VT_FLOAT) {
    target->literal.vt = VT_FLOAT;

    double left_val = (left_result.literal.vt == VT_FLOAT)
                          ? left_result.literal.fval
                          : (double)left_result.literal.ival;
    double right_val = (right_result.literal.vt == VT_FLOAT)
                           ? right_result.literal.fval
                           : (double)right_result.literal.ival;

    if (expr->binary.op == BO_ADD) {
      target->literal.fval = left_val + right_val;
    } else if (expr->binary.op == BO_SUB) {
      target->literal.fval = left_val - right_val;
    } else if (expr->binary.op == BO_MUL) {
      target->literal.fval = left_val * right_val;
    } else if (expr->binary.op == BO_DIV) {
      if (right_val == 0) {
        return new_div_by_zero_err();
      }
      target->literal.fval = left_val / right_val;
    }

  } else {
    target->literal.vt = VT_INT;

    if (expr->binary.op == BO_ADD) {
      target->literal.ival =
          left_result.literal.ival + right_result.literal.ival;
    } else if (expr->binary.op == BO_SUB) {
      target->literal.ival =
          left_result.literal.ival - right_result.literal.ival;
    } else if (expr->binary.op == BO_MUL) {
      target->literal.ival =
          left_result.literal.ival * right_result.literal.ival;
    } else if (expr->binary.op == BO_DIV) {
      if (right_result.literal.ival == 0) {
        return new_div_by_zero_err();
      }
      target->literal.ival =
          left_result.literal.ival / right_result.literal.ival;
    }
  }

  return NULL;
};

static lanalyzer_err_t *eval_expr(lanalyzer_t *ana, lexpr_t *expr,
                                  lexpr_t *target) {
  CHECK_NULL(ana, NULL);
  CHECK_NULL(expr, NULL);

  switch (expr->kind) {
  case LEXPK_LITERAL:
    if (expr->literal.vt == VT_UNTYPED) {
      return new_internal_err("Literal type is untyped");
    }

    target->literal.vt = expr->literal.vt;
    target->literal.ival = expr->literal.ival;
    target->literal.fval = expr->literal.fval;
    break;
  case LEXPK_VAR: {
    u32 name = hash_len(expr->variable.name.start, expr->variable.name.len);
    const lvar_stmt *var = scope_find(ana->current, name);
    if (var == NULL) {
      return new_sym_undef_err(expr->variable.name);
    }

    target->literal.vt = var->type;
    target->literal.ival = var->val->literal.ival;
    target->literal.fval = var->val->literal.fval;
  } break;
  case LEXPK_BINARY:
    return eval_binary(ana, expr, target);
    break;
  }
  return NULL;
};

lanalyzer_err_t *eval_var(lanalyzer_t *ana, lvar_stmt *var) {
  CHECK_NULL(ana, NULL);
  CHECK_NULL(var, NULL);

  if (var->type == VT_UNTYPED) {
    return new_internal_err("Variable type is untyped");
  }

  lexpr_t *final = new_lexpr(LEXPK_LITERAL);
  final->literal.vt = VT_UNTYPED;
  lanalyzer_err_t *err = eval_expr(ana, var->val, final);
  if (err != NULL) {
    lexpr_free(final);
    return err;
  }

  lexpr_free(var->val);
  var->val = final;

  return NULL;
}
