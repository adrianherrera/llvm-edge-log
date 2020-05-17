#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "edge-log.h"

static FILE *log_file = NULL;

__thread uint8_t __edge_type = EdgeUnknown;

void __edge_log(char *func_name) {
  static char *edge_strings[] = {" call", " return", " conditional branch",
                                 " unconditional branch", " switch",
                                 " unreachable", ""};
  void *ret_addr = __builtin_return_address(0);

  fprintf(log_file, "%s:%s edge to %p\n", func_name, edge_strings[__edge_type],
          ret_addr);
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
