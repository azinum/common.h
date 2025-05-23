// common.h
// stb-style header-only library
// thank you https://github.com/nothings/

// macros:
//  COMMON_IMPLEMENTATION
//  NO_STDLIB
//  NO_STDIO
//  NO_SIMD
//  NO_ASSERT
//  NO_TIMER
//  ASSERT
//  NPROC = 4
//  CACHELINESIZE = 64
//  BITS
//  MAX_PATH_LENGTH = 512
//  RANDOM_MAX
//  USE_STB_SPRINTF
//  SPRINTF_BUFFER_SIZE

#ifndef _COMMON_H
#define _COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h> // size_t
#include <stdarg.h> // va_list, va_arg

#if UINTPTR_MAX == 0xffffffffffffffff
  #define BITS 64
#elif UINTPTR_MAX == 0xffffffff
  #define BITS 32
#else
  #error "unsupported architecture"
#endif

// TODO: arm NEON
#ifndef NO_SIMD
  #if __SSE__
    #define USE_SIMD
    #define USE_SSE
    #include <xmmintrin.h>
  #endif
#endif

#ifdef MIN
  #undef MIN
#endif

#ifdef MAX
  #undef MAX
#endif

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(x, x_min, x_max) MIN(MAX(x_min, x), x_max)

#define EPS (1e-6)
#define SIGN(T, x) ((T)((x) > 0) - (T)((x) < 0))
#define ABS(T, x) (T)(SIGN(T, x) * (x))

// common type definitions
typedef double    f64;
typedef int64_t   i64;
typedef uint64_t  u64;
typedef float     f32;
typedef int32_t   i32;
typedef uint32_t  u32;
typedef int16_t   i16;
typedef uint16_t  u16;
typedef int8_t    i8;
typedef uint8_t   u8;

#ifndef bool
  #include <stdbool.h>
#endif

#ifndef true
  #define true 1
#endif

#ifndef false
  #define false 0
#endif

#ifndef NPROC
  #define NPROC (4)
#endif

#ifndef CACHELINESIZE
  #define CACHELINESIZE 64
#endif

#if __APPLE__ || __MACH__
  #define TARGET_APPLE
#elif __linux__
  #define TARGET_LINUX
#elif defined(_WIN32) || defined(_WIN64)
  #define WIN32_LEAN_AND_MEAN
  #define NOGDI
  #define NOUSER
  #define TARGET_WINDOWS
  #include <windows.h>
  #include <io.h>
#elif defined(TARGET_WASM)
  #define ssize_t intmax_t
#else
  #error "unsupported target"
#endif

#ifndef NO_TIMER
  #include <time.h>
  #include <sys/time.h> // gettimeofday

  #ifdef TARGET_WINDOWS
    #define TIMER_START() \
      struct timeval _timer_end = {0}; \
      struct timeval _timer_start = {0}; \
      gettimeofday(&_timer_start, NULL)

    #define TIMER_END() (gettimeofday(&_timer_end, NULL), ((((_timer_end.tv_sec - _timer_start.tv_sec) * 1000000.0f) + _timer_end.tv_usec) - (_timer_start.tv_usec)) / 1000000.0f)
  #else
    #define TIMER_START(...) \
      struct timespec _timer_end = {0}; \
      struct timespec _timer_start = {0}; \
      clock_gettime(CLOCK_MONOTONIC, &_timer_start); \
      __VA_ARGS__
    #define TIMER_END() (clock_gettime(CLOCK_MONOTONIC, &_timer_end), ((((_timer_end.tv_sec - _timer_start.tv_sec) * 1000000000.0f) + _timer_end.tv_nsec) - (_timer_start.tv_nsec)) / 1000000000.0f)
  #endif

#else
  #define TIMER_START(...)
  #define TIMER_END() 0
#endif


#ifdef __cplusplus
  #define RESTRICT
  #define COMMON_PUBLICDEC extern "C"
  #define COMMON_PUBLICDEF extern "C"
#else
  #define RESTRICT restrict
  #define COMMON_PUBLICDEC extern
  #define COMMON_PUBLICDEF
#endif

#ifndef MAX_PATH_LENGTH
  #define MAX_PATH_LENGTH 512
#endif

#define EXTRACTBIT(ith, byte) (byte & (1 << ith))
#define ALIGN(N, ALIGNMENT) ((N % ALIGNMENT) ? (N + ALIGNMENT - (N % ALIGNMENT)) : N)
#define LENGTH(ARR) (sizeof(ARR) / sizeof(ARR[0]))
#define return_defer(value) do { result = (value); goto defer; } while (0)
#define break_if(cond) if ((cond)) { break; }
#define continue_if(cond) if ((cond)) { continue; }
#define return_if(cond, ...) if ((cond)) { return __VA_ARGS__; }
typedef enum { Ok = 0, Error } Result;

const char* bool_str[] = { "false", "true" };

#define Kb(n) (n * 1024)
#define Mb(n) (Kb(n * 1024))
#define Gb(n) (Mb(n * 1024))

#define PI32  3.14159265359f
#define TAU32 6.28318530717f

#ifndef RANDOM_MAX
  #define RANDOM_MAX (size_t)((~0-1) >> 1)
#endif

#if defined(__has_builtin) // will not work for gcc versions < 10, even though the builtins may exist
  #if __has_builtin(__builtin_expect)
    #define LIKELY(x) (__builtin_expect(((x) != 0), 1))
    #define UNLIKELY(x) (__builtin_expect(((x) != 0), 0))
  #else
    #define LIKELY(x) (x)
    #define UNLIKELY(x) (x)
  #endif
#else
  #define LIKELY(x) (x)
  #define UNLIKELY(x) (x)
#endif

#define STR_(x) #x
#define STR(x) STR_(x)
#ifdef NO_STDLIB
  void* memset(void* p, i32 c, size_t n);
  void* memcpy(void* RESTRICT dest, const void* RESTRICT src, size_t n);
  size_t strlen(const char* s);
  size_t strnlen(const char* s, size_t maxlen);
  i32 strcmp(const char* s1, const char* s2);
  i32 strncmp(const char* s1, const char* s2, size_t n);
  char* strcpy(char* dest, const char* src);
  char* strncpy(char* dest, const char* src, size_t n);
#else
  #include <stdlib.h>
  #include <string.h> // memset, memcpy
#endif

#ifdef USE_STB_SPRINTF
  #define STB_WRAP(...) stb_##__VA_ARGS__
  #define printf(...)    stb_printf(__VA_ARGS__)
  #define dprintf(...)   stb_dprintf(__VA_ARGS__)
  #define sprintf(...)   stb_sprintf(__VA_ARGS__)
  #define snprintf(...)  stb_snprintf(__VA_ARGS__)

  #define vprintf(...)   stb_vprintf(__VA_ARGS__)
  #define vdprintf(...)  stb_vdprintf(__VA_ARGS__)
  #define vsprintf(...)  stb_vsprintf(__VA_ARGS__)
  #define vsnprintf(...) stb_vsnprintf(__VA_ARGS__)
#endif

#if defined(NO_STDIO) && defined(NO_STDLIB)
#else
  #include <stdio.h>
#endif

#ifdef USE_STB_SPRINTF
COMMON_PUBLICDEC i32 stb_printf(const char* fmt, ...);
COMMON_PUBLICDEC i32 stb_dprintf(i32 fd, const char* fmt, ...);
COMMON_PUBLICDEC i32 stb_sprintf(char* str, const char* fmt, ...);
COMMON_PUBLICDEC i32 stb_snprintf(char* str, size_t size, const char* fmt, ...);

COMMON_PUBLICDEC i32 stb_vprintf(const char* fmt, va_list argp);
COMMON_PUBLICDEC i32 stb_vdprintf(i32 fd, const char* fmt, va_list argp);
COMMON_PUBLICDEC i32 stb_vsprintf(char* str, const char* fmt, va_list argp);
COMMON_PUBLICDEC i32 stb_vsnprintf(char* str, size_t size, const char* fmt, va_list argp);
#endif // USE_STB_SPRINTF

#if defined(TARGET_LINUX) || defined(TARGET_APPLE)
  #include <unistd.h> // read, write, sleep
#elif defined(TARGET_WINDOWS)
  #define read _read
  #define write _write
  #define sleep(n) Sleep(n * 1000)
#elif defined(TARGET_WASM)
  extern i32 write(i32 fd, const void* buf, size_t size);
#endif

#ifdef TARGET_WASM
    #define DEBUG_BREAK
#else
  #if _MSC_VER
    #define DEBUG_BREAK __debugbreak()
  #elif defined(__has_builtin)
    #if __has_builtin(__builtin_debugtrap)
      #define DEBUG_BREAK __builtin_debugtrap()
    #elif __has_builtin(__builtin_trap)
      #define DEBUG_BREAK __builtin_trap()
    #endif
  #endif
#endif

#ifndef DEBUG_BREAK
  #include <signal.h>
  #if defined(SIGTRAP)
    #define DEBUG_BREAK raise(SIGTRAP)
  #else
    #define DEBUG_BREAK raise(SIGABRT)
  #endif
#endif

#ifndef __FUNCTION_NAME__
  #ifdef TARGET_WINDOWS
    #define __FUNCTION_NAME__ __FUNCTION__
  #else
    #define __FUNCTION_NAME__ __func__
  #endif
#endif

#ifndef STDIN_FILENO
  #if defined(TARGET_WINDOWS) && !defined(NO_STDIO)
    #define STDIN_FILENO _fileno(stdin)
  #else
    #define STDIN_FILENO 0
  #endif
#endif

#ifndef STDOUT_FILENO
  #if defined(TARGET_WINDOWS) && !defined(NO_STDIO)
    #define STDOUT_FILENO _fileno(stdout)
  #else
    #define STDOUT_FILENO 1
  #endif
#endif

#ifndef STDERR_FILENO
  #if defined(TARGET_WINDOWS) && !defined(NO_STDIO)
    #define STDERR_FILENO _fileno(stderr)
  #else
    #define STDERR_FILENO 2
  #endif
#endif

#ifndef EXIT_SUCCESS
  #define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
  #define EXIT_FAILURE 1
#endif

#ifndef NO_ASSERT
  #ifdef ASSERT
    // use custom assert macro
  #else
    #define ASSERT(...) \
      do { \
        if (!(__VA_ARGS__)) { \
          report_assert_failure(STDERR_FILENO, __FILE__, __LINE__, __FUNCTION_NAME__, #__VA_ARGS__); \
          DEBUG_BREAK; \
        } \
      } while (0)
  #endif
#else
  #define ASSERT(...)
#endif

#define NOT_IMPLEMENTED() ASSERT(!"not implemented")

#define INIT_ITEMS_SIZE 16
#define list_init(list, desired_size) \
  if ((list)->size < desired_size) { \
    (list)->size = desired_size; \
    (list)->items = memory_realloc((list)->items, (list)->size * sizeof(*(list)->items)); \
    ASSERT((list)->items != NULL && "out of memory"); \
  }

#define list_push(list, item) \
  if ((list)->count >= (list)->size) { \
    if ((list)->size == 0) { \
      (list)->size = INIT_ITEMS_SIZE; \
    } \
    else { \
      (list)->size *= 2; \
    } \
    (list)->items = memory_realloc((list)->items, (list)->size * sizeof(*(list)->items)); \
    ASSERT((list)->items != NULL && "out of memory"); \
  } \
  (list)->items[(list)->count++] = (item)

#define list_free(list) do { \
  memory_free((list)->items); \
  (list)->count = (list)->size = 0; \
} while (0)

void report_assert_failure(i32 fd, const char* filename, size_t line, const char* function_name, const char* message);
i32 is_terminal(i32 fd);
#ifdef TARGET_WINDOWS
  bool enable_vt100_mode(void);
#else
  #define enable_vt100_mode(...) true
#endif

#ifdef __cplusplus
}
#endif

#endif // _COMMON_H

#ifdef COMMON_IMPLEMENTATION

#ifdef NO_STDLIB

void* memset(void* p, i32 c, size_t n) {
  u8* it = (u8*)p;
  for (size_t i = 0; i < n; ++i) {
    *it++ = c;
  }
  return p;
}

void* memcpy(void* RESTRICT dest, const void* RESTRICT src, size_t n) {
  u8* dest_it = (u8*)dest;
  const u8* src_it = (const u8*)src;
  for (size_t i = 0; i < n; ++i) {
    *dest_it++ = *src_it++;
  }
  return dest;
}

size_t strlen(const char* s) {
  size_t n = 0;
  for (; *s++ != 0; ++n);
  return n;
}

size_t strnlen(const char* s, size_t maxlen) {
  size_t n = 0;
  for (; *s++ != 0 && n < maxlen; ++n);
  return n;
}

i32 strcmp(const char* s1, const char* s2) {
  for (; *s1 && *s1 == *s2; ++s1, ++s2);
  return *(u8*)s1 - *(u8*)s2;
}

i32 strncmp(const char* s1, const char* s2, size_t n) {
  size_t i = 0;
  for (; *s1 && *s1 == *s2 && i < n; ++s1, ++s2, ++i);
  if (i == n) {
    return 0;
  }
  return *(u8*)s1 - *(u8*)s2;
}

char* strcpy(char* dest, const char* src) {
  char* result = dest;
  for (;; ++dest, ++src) {
    *dest = *src;
    if (*dest == '\0') {
      return result;
    }
  }
  return NULL;
}

char* strncpy(char* dest, const char* src, size_t n) {
  size_t i = 0;
  for (; i < n && src[i] != '\0'; ++i) {
    dest[i] = src[i];
  }
  for (; i < n; ++i) {
    dest[i] = '\0';
  }
  return dest;
}

#endif // NO_STDLIB

#ifdef USE_STB_SPRINTF

#ifndef SPRINTF_BUFFER_SIZE
  #define SPRINTF_BUFFER_SIZE 4096
#endif

char sprintf_buffer[SPRINTF_BUFFER_SIZE] = {0};

COMMON_PUBLICDEF
i32 stb_printf(const char* fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  size_t n = STB_WRAP(vprintf)(fmt, argp);
  va_end(argp);
  return n;
}

COMMON_PUBLICDEF
i32 stb_dprintf(i32 fd,  const char* fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  size_t n = stb_vdprintf(fd, fmt, argp);
  va_end(argp);
  return n;
}

COMMON_PUBLICDEF
i32 stb_sprintf(char* str, const char* fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  size_t n = stbsp_vsprintf(str, fmt, argp);
  va_end(argp);
  return n;
}

COMMON_PUBLICDEF
i32 stb_snprintf(char* str, size_t size, const char* fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  size_t n = stbsp_vsnprintf(str, size, fmt, argp);
  va_end(argp);
  return n;
}

COMMON_PUBLICDEF
i32 stb_vprintf(const char* fmt, va_list argp) {
  return stb_vdprintf(STDOUT_FILENO, fmt, argp);
}

// TODO: write until fmt is processed completely
COMMON_PUBLICDEF
i32 stb_vdprintf(i32 fd, const char* fmt, va_list argp) {
  size_t n = stbsp_vsnprintf(sprintf_buffer, SPRINTF_BUFFER_SIZE, fmt, argp);
  return write(fd, sprintf_buffer, n);
}

COMMON_PUBLICDEF
i32 stb_vsprintf(char* str, const char* fmt, va_list argp) {
  return stbsp_vsprintf(str, fmt, argp);
}

COMMON_PUBLICDEF
i32 stb_vsnprintf(char* str, size_t size, const char* fmt, va_list argp) {
  return stbsp_vsnprintf(str, size, fmt, argp);
}

#endif // USE_STB_SPRINTF

void report_assert_failure(i32 fd, const char* filename, size_t line, const char* function_name, const char* message) {
  dprintf(STDERR_FILENO, "[assert-failed]: %s:%zu %s(): %s\n", filename, line, function_name, message);
}

i32 is_terminal(i32 fd) {
#if defined(TARGET_APPLE) || defined(TARGET_LINUX)
  return isatty(fd);
#elif defined(TARGET_WINDOWS)
  return _isatty(fd);
#else
  return 0;
#endif
}

#ifdef TARGET_WINDOWS
bool enable_vt100_mode(void) {
  HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD console_mode = 0;
  GetConsoleMode(handle, &console_mode);
  console_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  console_mode |= DISABLE_NEWLINE_AUTO_RETURN;
  return SetConsoleMode(handle, console_mode) != 0;
}
#endif

#endif // COMMON_IMPLEMENTATION

#undef COMMON_IMPLEMENTATION
