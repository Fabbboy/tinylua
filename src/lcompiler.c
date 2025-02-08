#include "lcompiler.h"
#include "llog.h"
#include "ltypes.h"
#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Types.h>
#include <stdbool.h>
#include <string.h>

static void compile_global(lcompiler_t *compiler, lvar_stmt *stmt) {
  CHECK_NULL(compiler, );
  CHECK_NULL(compiler->ast, );

  LLVMValueRef val = NULL;
  switch (stmt->val->kind) {
  case LEXPK_LITERAL: {
    switch (stmt->val->literal.vt) {
    case VT_INT: {
      val = LLVMConstInt(LLVMInt64Type(), stmt->val->literal.ival, 0);
    } break;
    case VT_FLOAT: {
      val = LLVMConstReal(LLVMDoubleType(), stmt->val->literal.fval);
    } break;
    default: {
      ERROR_LOG("Unknown literal type\n");
    } break;
    }
  } break;
  default:
    UNREACHABLE;
  }

  if (val == NULL) {
    ERROR_LOG("Failed to compile global\n");
    return;
  }

  LLVMTypeRef type = NULL;
  switch (stmt->type) {
  case VT_INT: {
    type = LLVMInt64Type();
  } break;
  case VT_FLOAT: {
    type = LLVMDoubleType();
  } break;
  default: {
    ERROR_LOG("Unknown type\n");
  } break;
  }

  if (type == NULL) {
    ERROR_LOG("Failed to compile global\n");
    return;
  }

  char *name = xmalloc(stmt->name.len + 1);
  memcpy(name, stmt->name.start, stmt->name.len);
  name[stmt->name.len] = '\0';
  LLVMValueRef global = LLVMAddGlobal(compiler->module, type, name);
  LLVMSetInitializer(global, val);
  if (stmt->link == LINK_EXTERNAL) {
    LLVMSetLinkage(global, LLVMExternalLinkage);
  } else {
    LLVMSetLinkage(global, LLVMInternalLinkage);
  }

  LLVMSetGlobalConstant(global, false);
  xfree(name);
}

void compiler_init(lcompiler_t *compiler, last_t *ast,
                   const char *module_name) {
  CHECK_NULL(compiler, );
  CHECK_NULL(ast, );

  compiler->ast = ast;
  compiler->context = LLVMContextCreate();
  compiler->module =
      LLVMModuleCreateWithNameInContext(module_name, compiler->context);
  compiler->builder = LLVMCreateBuilderInContext(compiler->context);
}

void compiler_compile(lcompiler_t *compiler) {
  CHECK_NULL(compiler, );
  CHECK_NULL(compiler->ast, );

  for (size_t i = 0; i < compiler->ast->globals.length; i++) {
    lvar_stmt *stmt = compiler->ast->globals.items[i];
    compile_global(compiler, stmt);
  }
};

void compiler_print(lcompiler_t *compiler) {
  CHECK_NULL(compiler, );
  CHECK_NULL(compiler->module, );
  LLVMDumpModule(compiler->module);
};

void compiler_write(lcompiler_t *compiler, const char *path,
                    const char *triple) {
  CHECK_NULL(compiler, );
  CHECK_NULL(compiler->module, );

  LLVMInitializeAllTargets();
  LLVMInitializeAllTargetMCs();
  LLVMInitializeAllTargetInfos();
  LLVMInitializeAllAsmPrinters();
  LLVMInitializeAllAsmParsers();

  char *err = NULL;
  LLVMTargetRef target;

  if (LLVMGetTargetFromTriple(triple, &target, &err)) {
    ERROR_LOG("Failed to get target: %s\n", err);
    LLVMDisposeMessage(err);
    return;
  }

  LLVMTargetMachineRef machine = LLVMCreateTargetMachine(
      target, triple, "generic", "", LLVMCodeGenLevelDefault, LLVMRelocDefault,
      LLVMCodeModelDefault);

  LLVMTargetDataRef data = LLVMCreateTargetDataLayout(machine);
  LLVMSetModuleDataLayout(compiler->module, data);

  LLVMTargetMachineEmitToFile(machine, compiler->module, path, LLVMObjectFile,
                              &err);

  if (err != NULL) {
    ERROR_LOG("Failed to write file: %s\n", err);
    LLVMDisposeMessage(err);
  }

  LLVMDisposeTargetData(data);
  LLVMDisposeTargetMachine(machine);
}

void compiler_free(lcompiler_t *compiler) {
  CHECK_NULL(compiler, );
  LLVMDisposeBuilder(compiler->builder);
  LLVMDisposeModule(compiler->module);
  LLVMContextDispose(compiler->context);
  LLVMShutdown();
};