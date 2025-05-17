// test_log.c

#include "test_common.h"

#define COMMON_IMPLEMENTATION
#include "common.h"

#define LOG_IMPL
#include "log.h"

i32 test(void);

i32 main(void) {
  return test();
}

i32 test(void) {
  log_init(true);
  const char* messages[MAX_LOG_TAG] = {
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
  };
  for (i32 i = 0; i < LENGTH(messages); ++i) {
    log_print(STDOUT_FILENO, i, "%s\n", messages[i]);
  }
  return EXIT_SUCCESS;
}
