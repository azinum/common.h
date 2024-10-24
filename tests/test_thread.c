// test_thread.c

#include "test_common.h"

#define COMMON_IMPLEMENTATION
#include "common.h"

#define THREAD_IMPLEMENTATION
#include "thread.h"

#define THREAD_COUNT NPROC * 4

typedef struct Handle {
  i32 value;
  i32 id;
} Handle;

i32 test(void);
void* hello(Handle* handle);

i32 main(void) {
  return test();
}

i32 test(void) {
  thread_init();

  verbose_printf("creating %d threads...\n", THREAD_COUNT);
  Handle threads[THREAD_COUNT];
  for (size_t i = 0; i < THREAD_COUNT; ++i) {
    Handle* handle = &threads[i];
    handle->value = i;
    handle->id = -1;
    handle->id = thread_create_v2(hello, handle);
    if (handle->id < 0) {
      return EXIT_FAILURE;
    }
  }
  verbose_printf("waiting for threads to join...\n");
  for (size_t i = 0; i < THREAD_COUNT; ++i) {
    Handle* handle = &threads[i];
    ASSERT(handle->id >= 0 && handle->id < MAX_THREADS);
    thread_join(handle->id);
    handle->id = -1;
  }
  verbose_printf("done\n");
  return EXIT_SUCCESS;
}

void* hello(Handle* handle) {
  (void)handle; // hide warning, for when VERBOSE is not defined
  for (i32 i = 0; i < 5; ++i) {
    verbose_printf("thread %2d: said hello\n", handle->id);
    sleep(0);
  }
  return NULL;
}
