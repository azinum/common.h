// test_random.c

#include "test_common.h"

#define COMMON_IMPLEMENTATION
#include "common.h"

#define RANDOM_IMPLEMENTATION
#include "random.h"

i32 test(void);

i32 main(void) {
  return test();
}

i32 test(void) {
  i32 exit_code[2] = {
    EXIT_FAILURE,
    EXIT_SUCCESS,
  };

  random_init(1234);
  return exit_code[random_number() == 20739851];
}
