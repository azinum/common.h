// test_thread_sync.c

#include "test_common.h"

#define COMMON_IMPLEMENTATION
#include "common.h"

#define THREAD_IMPLEMENTATION
#include "thread.h"

#define RANDOM_IMPLEMENTATION
#include "random.h"

#define THREAD_COUNT (NPROC * 2)
#define NUM_WORK_PER_THREAD 4

#include <math.h>

typedef struct Shared {
  Barrier barrier;
} Shared;

typedef struct Handle {
  Shared* shared;
  i32 count;
  i32 id;
} Handle;

i32 test(void);
void* work(Handle* data);
void sleep_ms(size_t ms);

i32 main(void) {
  return test();
}

i32 test(void) {
  random_init(1234);
  thread_init();
  Shared shared = {
    .barrier = barrier_new(THREAD_COUNT),
  };

  verbose_printf("creating %d threads...\n", THREAD_COUNT);
  Handle handles[THREAD_COUNT] = {0};
  for (size_t i = 0; i < THREAD_COUNT; ++i) {
    Handle* handle = &handles[i];
    handle->shared = &shared;
    handle->count = 0;
    handle->id = -1;
    if ((handle->id = thread_create_v2((void*)work, handle)) < 0) {
      return EXIT_FAILURE;
    }
  }
  verbose_printf("waiting for threads to join...\n");
  for (size_t i = 0; i < THREAD_COUNT; ++i) {
    Handle* handle = &handles[i];
    ASSERT(handle->id >= 0 && handle->id < MAX_THREADS);
    thread_join(handle->id);
    handle->id = -1;
  }
  verbose_printf("done\n");
  return EXIT_SUCCESS;
}

void* work(Handle* data) {
  verbose_printf("%d: start\n", data->id);
  size_t ms = 300 + random_number() % 700;
  for (i32 work_index = 0; work_index < NUM_WORK_PER_THREAD; ++work_index) {
    verbose_printf("%d: waiting for %g seconds\n", data->id, ms/1000.0f);
    sleep_ms(ms);
    verbose_printf("%d: wait for barrier (%d)\n", data->id, work_index);
    barrier_wait(&data->shared->barrier);
  }
  verbose_printf("%d: end\n", data->id);
  return NULL;
}

void sleep_ms(size_t ms) {
#ifdef TARGET_WINDOWS
  Sleep(ms);
#else
  struct timespec ts = {
    .tv_sec = 0,
    .tv_nsec = (time_t)(ms * 1e+6),
  };
  nanosleep(&ts, NULL);
#endif
}
