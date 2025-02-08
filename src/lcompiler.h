#pragma once

#include "last.h"
typedef struct {
  last_t *ast;
} lcompiler_t;

void compiler_init(lcompiler_t *compiler, last_t *ast);