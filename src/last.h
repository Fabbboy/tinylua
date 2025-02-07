#pragma once

#include "llex.h"
#include "ltypes.h"

typedef list_t globals_t;

typedef enum { LEXPK_LITERAL } lexpr_kind_t;

typedef enum { VT_UNTYPED, VT_FLOAT, VT_INT } value_type_t;

typedef struct {
  lexpr_kind_t kind;
  union {
    struct {
      value_type_t vt;
      union {
        f64 fval;
        s64 ival;
      };
    } literal;
  };
} lexpr_t;

lexpr_t *new_lexpr(lexpr_kind_t kind);
void lexpr_free(lexpr_t *expr);

typedef struct {
  tok_t name;
  lexpr_t *val;
} lvar_stmt;

lvar_stmt *new_lvar_stmt(tok_t name, lexpr_t *val);
void lvar_stmt_free(void *item);

typedef struct {
  globals_t globals;
} last_t;

last_t new_ast();
void ast_free(last_t *ast);