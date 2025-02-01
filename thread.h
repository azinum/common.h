// thread.h

// macros:
//  THREAD_IMPLEMENTATION
//  MAX_THREADS = 64

#ifndef _THREAD_H
#define _THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef struct Ticket {
  volatile size_t ticket;
  volatile size_t serving;
} Ticket;

typedef struct Barrier {
  volatile size_t count;
  volatile size_t waiting;
  volatile size_t thread_count;
  Ticket fence;
} Barrier;

#ifndef MAX_THREADS
  #define MAX_THREADS 64
#endif

typedef void* (*thread_func_sig)(void*);
#if defined(TARGET_LINUX) || defined(TARGET_APPLE)
  #include <pthread.h>
  #include <errno.h>
  typedef struct Thread {
    pthread_t thread;
    thread_func_sig thread_func;
    void* data;
    bool active;
  } Thread;
#elif defined(TARGET_WINDOWS)
  typedef struct Thread_data {
    thread_func_sig thread_func;
    void* data;
  } Thread_data;
  typedef struct Thread {
    HANDLE handle;
    DWORD id;
    Thread_data data;
    bool active;
  } Thread;
#else
  #error "platform not supported"
#endif

// TODO: thread pool
typedef struct Thread_state {
  Thread threads[MAX_THREADS];
  Ticket mutex; // lock when creating new threads
} Thread_state;

#endif // _THREAD_H

COMMON_PUBLICDEC void thread_init(void);
COMMON_PUBLICDEC i32 thread_create(thread_func_sig thread_func, void* data);
COMMON_PUBLICDEC i32 thread_create_v2(void* thread_func, void* data);
COMMON_PUBLICDEC Result thread_join(i32 id);
COMMON_PUBLICDEC void thread_exit(void);

COMMON_PUBLICDEC u32 thread_get_id(void); // TODO: map real thread ids to internal ones
COMMON_PUBLICDEC size_t atomic_fetch_add(volatile size_t* target, size_t value);
COMMON_PUBLICDEC size_t atomic_fetch_sub(volatile size_t* target, size_t value);
COMMON_PUBLICDEC size_t atomic_load(volatile size_t* target);
COMMON_PUBLICDEC void atomic_store(volatile size_t* target, size_t value);
COMMON_PUBLICDEC size_t atomic_compare_exchange(volatile size_t* target, size_t value, size_t expected);
COMMON_PUBLICDEC Ticket ticket_mutex_new(void);
COMMON_PUBLICDEC void ticket_mutex_begin(Ticket* mutex);
COMMON_PUBLICDEC void ticket_mutex_end(Ticket* mutex);
COMMON_PUBLICDEC void spin_wait(void);
COMMON_PUBLICDEC Barrier barrier_new(size_t thread_count);
COMMON_PUBLICDEC void barrier_wait(Barrier* barrier);

#ifdef __cplusplus
}
#endif

#ifdef THREAD_IMPLEMENTATION

static char* thread_error_string = (char*)"";

static Thread_state thread_state = {};

#if defined(TARGET_LINUX) || defined(TARGET_APPLE)

COMMON_PUBLICDEF
void thread_init(void) {
  for (size_t i = 0; i < MAX_THREADS; ++i) {
    Thread* t      = &thread_state.threads[i];
    t->thread      = 0,
    t->thread_func = NULL;
    t->data        = NULL;
    t->active      = false;
  }
  thread_state.mutex = ticket_mutex_new();
}

COMMON_PUBLICDEF
i32 thread_create(thread_func_sig thread_func, void* data) {
  return thread_create_v2((void*)thread_func, data);
}

COMMON_PUBLICDEF
i32 thread_create_v2(void* thread_func, void* data) {
  ticket_mutex_begin(&thread_state.mutex); // use mutex here so there will be no thread id collisions
  Thread* thread = NULL;
  i32 id = -1;
  for (size_t i = 0; i < MAX_THREADS; ++i) {
    Thread* t = &thread_state.threads[i];
    if (t->active == false) {
      thread = t;
      id = i;
      break;
    }
  }
  if (thread) {
    thread->thread_func = (thread_func_sig)thread_func;
    thread->data        = data;
    thread->active      = true;
    i32 err = pthread_create(
      &thread->thread,
      NULL,
      (thread_func_sig)thread_func,
      data
    );
    if (!err) {
      ticket_mutex_end(&thread_state.mutex);
      return id;
    }
    switch (err) {
      case EAGAIN: thread_error_string = (char*)"insufficient resources to create new thread"; break;
      case EINVAL: thread_error_string = (char*)"invalid settings in attr"; break;
      case EPERM:  thread_error_string = (char*)"no permission to set the scheduling policy and parameters specified in attr"; break;
      default: break;
    }
  }
  ticket_mutex_end(&thread_state.mutex);
  return id;
}

COMMON_PUBLICDEF
Result thread_join(i32 id) {
  ASSERT(id >= 0 && id < MAX_THREADS);
  Thread* thread = &thread_state.threads[id];
  i32 err = pthread_join(thread->thread, NULL);
  if (!err) {
    thread->active = false;
    return Ok;
  }
  switch (err) {
    case EDEADLK: thread_error_string = (char*)"a deadlock was detected"; break;
    case EINVAL:  thread_error_string = (char*)"thread is not a joinable thread"; break;
    case ESRCH:   thread_error_string = (char*)"no thread with the ID thread could be found"; break;
    default: break;
  }
  return Ok;
}

COMMON_PUBLICDEF
void thread_exit(void) {
  pthread_exit(NULL);
}

#elif defined(TARGET_WINDOWS)

static DWORD WINAPI win_thread_func_wrapper(LPVOID data);

DWORD WINAPI win_thread_func_wrapper(LPVOID data) {
  Thread_data* thread_data = (Thread_data*)data;
  thread_data->thread_func(thread_data->data);
  return 1;
}

COMMON_PUBLICDEF
void thread_init(void) {
  for (size_t i = 0; i < MAX_THREADS; ++i) {
    Thread* t           = &thread_state.threads[i];
    t->id               = 0,
    t->data.thread_func = NULL;
    t->data.data        = NULL;
    t->active           = false;
  }
}

COMMON_PUBLICDEF
i32 thread_create(thread_func_sig thread_func, void* data) {
  return thread_create_v2(thread_func, data);
}

COMMON_PUBLICDEF
i32 thread_create_v2(void* thread_func, void* data) {
  ticket_mutex_begin(&thread_state.mutex);
  Thread* thread = NULL;
  i32 id = -1;
  for (size_t i = 0 ; i < MAX_THREADS; ++i) {
    Thread* t = &thread_state.threads[i];
    if (t->active == false) {
      thread = t;
      id = i;
      break;
    }
  }
  if (thread) {
    thread->data.thread_func = (thread_func_sig)thread_func;
    thread->data.data = data;
    thread->active = true;
    Thread_data* thread_data = &thread->data;
    thread->handle = CreateThread(
      NULL,
      0,
      win_thread_func_wrapper,
      thread_data,
      0,
      &thread->id
    );
    if (!thread->handle) {
      thread_error_string = (char*)"failed to create thread";
      ticket_mutex_end(&thread_state.mutex);
      return -1;
    }
    ticket_mutex_end(&thread_state.mutex);
    return id;
  }
  ticket_mutex_end(&thread_state.mutex);
  return -1;
}

COMMON_PUBLICDEF
Result thread_join(i32 id) {
  ASSERT(id >= 0 && id < MAX_THREADS);
  Thread* thread = &thread_state.threads[id];
  sleep(0);
  WaitForSingleObject(thread->handle, INFINITE);
  CloseHandle(thread->handle);
  return Ok;
}

COMMON_PUBLICDEF
void thread_exit(void) {
  // nothing to do
}

#endif


// TODO: cross-platform
COMMON_PUBLICDEF
u32 thread_get_id(void) {
  u32 thread_id = 0;
#if defined(__APPLE__) && defined(__x86_64__)
  asm("mov %%gs:0x00, %0" : "=r" (thread_id));
#elif defined(__i386__)
  asm("mov %%gs:0x08, %0" : "=r" (thread_id));
#elif defined(__x86_64__)
  asm("mov %%fs:0x10, %0" : "=r" (thread_id));
#else
  // #error "thread_get_id: Unsupported architecture."
  NOT_IMPLEMENTED();
#endif
  return thread_id;
}

COMMON_PUBLICDEF
inline size_t atomic_fetch_add(volatile size_t* target, size_t value) {
  return __atomic_fetch_add(target, value, __ATOMIC_SEQ_CST);
}

COMMON_PUBLICDEF
inline size_t atomic_fetch_sub(volatile size_t* target, size_t value) {
  return __atomic_fetch_sub(target, value, __ATOMIC_SEQ_CST);
}

COMMON_PUBLICDEF
inline size_t atomic_load(volatile size_t* target) {
  return __atomic_load_n(target, __ATOMIC_SEQ_CST);
}

COMMON_PUBLICDEF
inline void atomic_store(volatile size_t* target, size_t value) {
  __atomic_store_n(target, value, __ATOMIC_SEQ_CST);
}

COMMON_PUBLICDEF
inline size_t atomic_compare_exchange(volatile size_t* target, size_t value, size_t expected) {
  return __atomic_compare_exchange(target, &expected, &value, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

COMMON_PUBLICDEF
inline Ticket ticket_mutex_new(void) {
  return (Ticket) {
    .ticket = 0,
    .serving = 0,
  };
}

COMMON_PUBLICDEF
inline void ticket_mutex_begin(Ticket* mutex) {
  size_t ticket = atomic_fetch_add(&mutex->ticket, 1);
  while (ticket != mutex->serving) {
    spin_wait();
  };
}

COMMON_PUBLICDEF
inline void ticket_mutex_end(Ticket* mutex) {
  atomic_fetch_add(&mutex->serving, 1);
}

COMMON_PUBLICDEF
inline void spin_wait(void) {
#ifdef USE_SIMD
  _mm_pause();
#else
  sleep(0);
#endif
}

COMMON_PUBLICDEF
Barrier barrier_new(size_t thread_count) {
  return (Barrier) {
    .count = 0,
    .waiting = 0,
    .thread_count = thread_count,
    .fence = ticket_mutex_new(),
  };
}

COMMON_PUBLICDEF
void barrier_wait(Barrier* barrier) {
  atomic_fetch_add(&barrier->count, 1);
  atomic_fetch_add(&barrier->waiting, 1);
  while (atomic_load(&barrier->count) < barrier->thread_count) {
    spin_wait();
  }
  atomic_fetch_sub(&barrier->waiting, 1);
  ticket_mutex_begin(&barrier->fence);
  if (atomic_load(&barrier->waiting) == 0) {
    atomic_store(&barrier->count, 0);
  }
  ticket_mutex_end(&barrier->fence);
}

#endif // THREAD_IMPLEMENTATION
#undef THREAD_IMPLEMENTATION
