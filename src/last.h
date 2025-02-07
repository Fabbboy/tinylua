#pragma once

#include "llex.h"
#include "ltypes.h"
typedef enum { LEXPK_LITERAL } lexpr_kind_t;

typedef struct {
  lexpr_kind_t kind;
  union {
    f64 fval;
    s64 ival;
  };
} lexpr_t;

typedef struct {
  tok_t name;
  lexpr_t val;
} lvar_assign;

typedef list_t globals_t;

typedef struct {
  globals_t globals;
} last_t;

last_t new_ast();
void ast_free(last_t* ast);