#pragma once

#include "ltypes.h"

typedef enum {
  LIRT_INT,
  LIRT_FLOAT,
} lir_type_t;

typedef enum {
  LINK_EXTERNAL,
  LINK_INTERNAL,
} linkage_t;

typedef struct {
  lir_type_t type;
  union {
    s64 ival;
    f64 fval;
  };
} lir_constant_t;

lir_constant_t new_lir_constant_int(s64 val);
lir_constant_t new_lir_constant_float(f64 val);

typedef struct {
  char *name;
  linkage_t link;
  lir_constant_t val;
} lir_global_t;

lir_global_t *new_lir_global(const char *name, linkage_t link,
                             lir_constant_t val);
void lir_global_free(void *global);

typedef list_t lir_global_list_t;

typedef struct {
  lir_global_list_t globals;
} lir_module_t;

lir_module_t *new_lir_module();
void lir_module_free(lir_module_t *mod);