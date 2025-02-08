#pragma once

#include "lir.h"
typedef struct {
  lir_module_t module;
} lcompiler_t;

void compiler_init(lcompiler_t *compiler);
void compiler_free(lcompiler_t *compiler);