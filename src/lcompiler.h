#pragma once

#include "last.h"
#include <llvm-c/Types.h>
typedef struct {
  last_t *ast;
  LLVMModuleRef module;
  LLVMBuilderRef builder;
  LLVMContextRef context;
} lcompiler_t;

void compiler_init(lcompiler_t *compiler, last_t *ast, const char *module_name);
void compiler_compile(lcompiler_t *compiler);
void compiler_print(lcompiler_t *compiler);
void compiler_write(lcompiler_t *compiler, const char *path,
                    const char *triple);
void compiler_free(lcompiler_t *compiler);