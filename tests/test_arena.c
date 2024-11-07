// test_arena.c

#include "test_common.h"

#define COMMON_IMPLEMENTATION
#include "common.h"

#define ARENA_IMPLEMENTATION
#include "arena.h"

i32 test(void);

i32 main(void) {
  return test();
}

i32 test(void) {
  i32 result = EXIT_SUCCESS;
  i32 a_count = 8;
  i32* a = NULL;
  i32 b_count = 12;
  i32* b = NULL;
  Arena arena = arena_new(1024);
  a = (i32*)arena_alloc(&arena, a_count * sizeof(i32));
  b = (i32*)arena_alloc(&arena, b_count * sizeof(i32));
  for (i32 i = 0; i < a_count; ++i) {
    a[i] = i * 2;
  }
  for (i32 i = 0; i < b_count; ++i) {
    b[i] = i * 3;
  }
  for (i32 i = 0; i < a_count; ++i) {
    verbose_printf("a[%d] = %d\n", i, a[i]);
    if (a[i] != i * 2) {
      result = EXIT_FAILURE;
      break;
    }
  }
  for (i32 i = 0; i < b_count; ++i) {
    verbose_printf("b[%d] = %d\n", i, b[i]);
    if (b[i] != i * 3) {
      result = EXIT_FAILURE;
      break;
    }
  }
  arena_free(&arena);
  return result;
}
