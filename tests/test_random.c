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

  i32 n = random_number();
  random_freeze(2);
  if (random_number() != n || random_number() != n) {
    return EXIT_FAILURE;
  }
  if (random_number() == n) {
    return EXIT_FAILURE;
  }

  random_init(1234);
  return exit_code[random_number() == 20739851];
}
