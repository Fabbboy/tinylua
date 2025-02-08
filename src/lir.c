#include "lir.h"
#include "llog.h"
#include "ltypes.h"

lir_constant_t new_lir_constant_int(s64 val) {
  lir_constant_t c;
  c.type = LIRT_INT;
  c.ival = val;
  return c;
};

lir_constant_t new_lir_constant_float(f64 val) {
  lir_constant_t c;
  c.type = LIRT_FLOAT;
  c.fval = val;
  return c;
};

lir_global_t *new_lir_global(const char *name, linkage_t link,
                             lir_constant_t val) {
  CHECK_NULL(name, NULL);
  lir_global_t *global = xmalloc(sizeof(lir_global_t));
  global->name = xstrdup(name);
  global->link = link;
  global->val = val;
  return global;
};

void lir_global_free(void *global) {
  CHECK_NULL(global, );
  lir_global_t *g = global;
  xfree(g->name);
  xfree(global);
};

lir_module_t new_lir_module() {
  lir_module_t mod;
  mod.globals = new_list(8);
  return mod;
};

void lir_module_free(lir_module_t *mod) {
  CHECK_NULL(mod, );
  list_free(&mod->globals, lir_global_free);
};