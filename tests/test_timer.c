// test_timer.c

#include "test_common.h"

#define COMMON_IMPLEMENTATION
#include "common.h"

i32 test(void);

i32 main(void) {
  return test();
}

i32 test(void) {
  TIMER_START();
  sleep(1);
  f32 dt = TIMER_END();
  verbose_printf("time elapsed: %g seconds\n", dt);
  return EXIT_SUCCESS;
}
