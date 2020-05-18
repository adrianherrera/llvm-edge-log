#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "edge-log.h"

static FILE *log_file = NULL;

void __edge_log(const char *file, const char *func, int32_t line,
                enum EdgeType edge_type) {
  static const char *edge_strings[] = {
      "function call",        "function return", "conditional branch",
      "unconditional branch", "switch",          "unreachable",
      "unknown edge"};
  void *ret_addr = __builtin_return_address(0);

  if (line < 0) {
    fprintf(log_file, "[%s:%s:?] %s @%p\n", file, func, edge_strings[edge_type],
            ret_addr);
  } else {
    fprintf(log_file, "[%s:%s:%d] %s @%p\n", file, func, line,
            edge_strings[edge_type], ret_addr);
  }

  fflush(log_file);
}

__attribute__((constructor)) void __edge_log_init(void) {
  if (log_file) {
    return;
  }

  char *log_path = getenv(EDGE_LOG_ENV);
  if (!log_path) {
    log_file = stderr;
  }

  log_file = fopen(log_path, "w");
  if (!log_file) {
    log_file = stderr;
  }
}
