#pragma once

#include "last.h"
#include "llex.h"
#include "ltypes.h"
typedef enum {
  AEK_SYM_UNDEFINED,
  AEK_TYPE_MISMATCH,
} lanalyzer_err_kind_t;

typedef struct {
  lanalyzer_err_kind_t kind;
  union {
    struct {
      tok_t sym;
    } sym_undef;
    struct {
      lvalue_type_t expected;
      lvalue_type_t got;
    } type_mismatch;
  };
} lanalyzer_err_t;

lanalyzer_err_t *new_sym_undef_err(tok_t sym);
lanalyzer_err_t *new_type_mismatch_err(lvalue_type_t expected, lvalue_type_t got);
void analyzer_err_string(lanalyzer_err_t *err, fbuffer_t *buf);
void analyzer_err_free(void *err);

typedef list_t laerr_list_t;
typedef struct lscope_t lscope_t;

typedef struct {
  u32 name;
  const lvar_stmt *variable;
} lnamend_t;

lnamend_t *new_namend(u32 name, const lvar_stmt *var);
void namend_free(void *namend);

struct lscope_t {
  lscope_t *parent;
  list_t namend;
};

lscope_t *new_scope(lscope_t *parent);
void scope_add(lscope_t *scope, lvar_stmt *var);
const lvar_stmt *scope_find(lscope_t *scope, u32 name);
void scope_free(lscope_t *scope);

typedef struct {
  laerr_list_t errs;
  lscope_t *global;
  lscope_t *current;
} lanalyzer_t;

void analyzer_init(lanalyzer_t *analyzer);
void ana_infer_type(lanalyzer_t *ana, lvar_stmt *var);
void analyzer_free(lanalyzer_t *analyzer);