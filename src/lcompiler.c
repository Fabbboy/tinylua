#include "lcompiler.h"

void compiler_init(lcompiler_t *compiler) {
  compiler->module = new_lir_module();
};

void compiler_free(lcompiler_t *compiler) {
  lir_module_free(&compiler->module);
};