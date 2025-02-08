#pragma once

#include "last.h"
#include "ltypes.h"
typedef enum {
  TYPE_MISMATCH,
} lanalyzer_err_kind_t;

typedef struct {
  lanalyzer_err_kind_t kind;
  union {};
} lanalyzer_err_t;

typedef list_t laerr_list_t;

typedef struct {
  laerr_list_t errs;
} lanalyzer_t;

void analyzer_init(lanalyzer_t *analyzer);
void ana_infer_type(lanalyzer_t* ana, lvar_stmt* var);
void analyzer_free(lanalyzer_t *analyzer);