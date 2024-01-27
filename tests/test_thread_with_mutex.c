// test.c

#define STB_SPRINTF_IMPLEMENTATION
#define USE_STB_SPRINTF
#include "stb_sprintf.h"

#define COMMON_IMPLEMENTATION
#include "common.h"

#define THREAD_IMPLEMENTATION
#include "thread.h"

#include "test_common.h"

#define THREAD_COUNT (NPROC * 4)
#define NUM_WORK_PER_THREAD 25
#define TOTAL_WORK (THREAD_COUNT * NUM_WORK_PER_THREAD)

typedef struct Shared {
  Ticket mutex;
  i32 value;
} Shared;

typedef struct Handle {
  Shared* shared;
  i32 work_count; // number of times a thread did work, it is used to calculate the work balance between threads
  i32 id;
} Handle;

i32 test(void);
void* hello(void* data);

i32 main(void) {
  return test();
}

i32 test(void) {
  thread_init();
  Shared shared = {
    .value = 0,
    .mutex = ticket_mutex_new(),
  };

  verbose_printf("creating %d threads...\n", THREAD_COUNT);
  Handle threads[THREAD_COUNT];
  for (size_t i = 0; i < THREAD_COUNT; ++i) {
    Handle* handle = &threads[i];
    handle->shared = &shared;
    handle->work_count = 0;
    handle->id = -1;
    if ((handle->id = thread_create(hello, handle)) < 0) {
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
  ASSERT(shared.value == TOTAL_WORK);
  verbose_printf("done\n");
  return EXIT_SUCCESS;
}

void* hello(void* data) {
  Handle* handle = (Handle*)data;
  for (i32 i = 0; i < NUM_WORK_PER_THREAD; ++i) {
    const i32 increment = 1;
    ticket_mutex_begin(&handle->shared->mutex);
    i32 expect = handle->shared->value + increment;
    handle->shared->value += increment;
    ASSERT(expect == handle->shared->value);
    verbose_printf("thread %2d: modified shared value from %d -> %d\n", handle->id, handle->shared->value - increment, handle->shared->value);
    ticket_mutex_end(&handle->shared->mutex);
  }
  return NULL;
}
