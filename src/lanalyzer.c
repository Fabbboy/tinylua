#include "lanalyzer.h"
#include "llog.h"

void analyzer_init(lanalyzer_t *analyzer) {
  CHECK_NULL(analyzer, );
  analyzer->errs = new_list(8);
};

void analyzer_free(lanalyzer_t *analyzer) {
  CHECK_NULL(analyzer, );
  list_free(&analyzer->errs, NULL);
};