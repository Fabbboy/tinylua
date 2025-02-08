#pragma once

#include "llex.h"
#include "ltypes.h"

typedef list_t globals_t;
typedef struct lexpr_t lexpr_t;

typedef enum {
  LEXPK_LITERAL,
  LEXPK_BINARY,
} lexpr_kind_t;

typedef enum {
  BO_ADD,
  BO_SUB,
  BO_MUL,
  BO_DIV,
} bin_op_t;

struct lexpr_t {
  lexpr_kind_t kind;
  union {
    struct {
      lvalue_type_t vt;
      union {
        f64 fval;
        s64 ival;
      };
    } literal;
    struct {
      bin_op_t op;
      lexpr_t *left;
      lexpr_t *right;
    } binary;
  };
};

lexpr_t *new_lexpr(lexpr_kind_t kind);
void lexpr_free(lexpr_t *expr);
void lexpr_string(lexpr_t *expr, fbuffer_t *buf);

typedef enum {
  LINK_EXTERNAL,
  LINK_INTERNAL,
} linkage_t;

typedef struct {
  tok_t name;
  lexpr_t *val;
  linkage_t link;
  lvalue_type_t type;
} lvar_stmt;

lvar_stmt *new_lvar_stmt(tok_t name, lexpr_t *val, linkage_t link,
                         lvalue_type_t type);
void lvar_stmt_free(void *item);
void lvar_stmt_string(lvar_stmt *stmt, fbuffer_t *buf);

typedef struct {
  globals_t globals;
} last_t;

last_t new_ast();
void ast_free(last_t *ast);