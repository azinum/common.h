// test.c

#define STB_SPRINTF_IMPLEMENTATION
#define USE_STB_SPRINTF
#include "stb_sprintf.h"

#define COMMON_IMPLEMENTATION
#include "common.h"

#define THREAD_IMPLEMENTATION
#include "thread.h"

Ticket mutex;

typedef struct Handle {
  i32 value;
  i32 id;
} Handle;

void test_threads(void);
void* hello(Handle* data);

i32 main(void) {
  test_threads();
  return 0;
}

void test_threads(void) {
  thread_init();
  mutex = ticket_mutex_new();

#define THREAD_COUNT 16
  printf("creating %d threads...\n", THREAD_COUNT);
  Handle threads[THREAD_COUNT];
  for (size_t i = 0; i < THREAD_COUNT; ++i) {
    Handle* handle = &threads[i];
    handle->value = i;
    handle->id = -1;
    handle->id = thread_create(hello, handle);
  }
  printf("waiting for threads to join...\n");
  for (size_t i = 0; i < THREAD_COUNT; ++i) {
    Handle* handle = &threads[i];
    ASSERT(handle->id >= 0 && handle->id < MAX_THREADS);
    thread_join(handle->id);
    handle->id = -1;
  }
  printf("done\n");
}

void* hello(Handle* handle) {
  for (size_t i = 0; i < 5; ++i) {
    printf("%zu: hello from %d\n", i, handle->value);
    sleep(1);
  }
  return NULL;
}
