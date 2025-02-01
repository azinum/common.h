// buffer.h

#ifndef _BUFFER_H
#define _BUFFER_H

#include <stddef.h> // size_t

#ifdef __cplusplus
  #define RESTRICT
  #define BUFFER_PUBLICDEC extern "C"
  #define BUFFER_PUBLICDEF extern "C"
#else
  #define RESTRICT restrict
  #define BUFFER_PUBLICDEC extern
  #define BUFFER_PUBLICDEF
#endif

#ifndef BUFFER_INIT_SIZE
  #define BUFFER_INIT_SIZE 16
#endif

#ifndef BUFFER_MEMORY_MALLOC
  #define BUFFER_MEMORY_MALLOC malloc
#endif
#ifndef BUFFER_MEMORY_CALLOC
  #define BUFFER_MEMORY_CALLOC calloc
#endif
#ifndef BUFFER_MEMORY_REALLOC
  #define BUFFER_MEMORY_REALLOC realloc
#endif
#ifndef BUFFER_MEMORY_FREE
  #define BUFFER_MEMORY_FREE free
#endif

#ifndef BUFFER_ASSERT
  #define BUFFER_ASSERT(a) assert(a)
#endif

void* (*buffer_memory_malloc)(size_t size)           = BUFFER_MEMORY_MALLOC;
void* (*buffer_memory_calloc)(size_t n, size_t size) = BUFFER_MEMORY_CALLOC;
void* (*buffer_memory_realloc)(void* p, size_t size) = BUFFER_MEMORY_REALLOC;
void  (*buffer_memory_free)(void* p)                 = BUFFER_MEMORY_FREE;

typedef struct Buffer {
  char* data;
  size_t count; // how many bytes are in use?
  size_t size;  // number of bytes allocated
} Buffer;

BUFFER_PUBLICDEC Buffer buffer_new(size_t size);
BUFFER_PUBLICDEC Buffer buffer_new_from_str(const char* str);
BUFFER_PUBLICDEC Buffer buffer_new_from_fmt(size_t size, const char* fmt, ...);
BUFFER_PUBLICDEC void buffer_from_fmt(Buffer* buffer, size_t size, const char* fmt, ...);
BUFFER_PUBLICDEC void buffer_reset(Buffer* buffer);
BUFFER_PUBLICDEC void buffer_append(Buffer* buffer, char byte);
BUFFER_PUBLICDEC void buffer_insert(Buffer* buffer, char byte, size_t index);
BUFFER_PUBLICDEC void buffer_erase(Buffer* buffer, size_t index);
BUFFER_PUBLICDEC void buffer_free(Buffer* buffer);
BUFFER_PUBLICDEC Buffer buffer_new_from_fd(int fd);

#endif // _BUFFER_H

#ifdef BUFFER_IMPL

BUFFER_PUBLICDEF
Buffer buffer_new(size_t size) {
  Buffer buffer = (Buffer) {
    .data = buffer_memory_calloc(size, 1),
    .count = 0,
    .size = size,
  };
  BUFFER_ASSERT(buffer.data != NULL || size == 0);
  return buffer;
}

BUFFER_PUBLICDEF
Buffer buffer_new_from_str(const char* str) {
  size_t length = strlen(str);
  Buffer buffer = (Buffer) {
    .data = buffer_memory_malloc(length),
    .count = length,
    .size = length,
  };
  memcpy(buffer.data, str, length);
  return buffer;
}

BUFFER_PUBLICDEF
Buffer buffer_new_from_fmt(size_t size, const char* fmt, ...) {
  Buffer buffer = buffer_new(size);

  va_list argp;
  va_start(argp, fmt);
  buffer.count = vsnprintf((char*)buffer.data, size, fmt, argp);
  va_end(argp);

  return buffer;
}

BUFFER_PUBLICDEF
void buffer_from_fmt(Buffer* buffer, size_t size, const char* fmt, ...) {
  if (size <= buffer->size) {
    va_list argp;
    va_start(argp, fmt);
    buffer->count = vsnprintf((char*)buffer->data, size, fmt, argp);
    va_end(argp);
    return;
  }
  buffer_free(buffer);
  *buffer = buffer_new(size);
  va_list argp;
  va_start(argp, fmt);
  buffer->count = vsnprintf((char*)buffer->data, size, fmt, argp);
  va_end(argp);
}

BUFFER_PUBLICDEF
void buffer_reset(Buffer* buffer) {
  buffer->count = 0;
}

BUFFER_PUBLICDEF
void buffer_append(Buffer* buffer, char byte) {
  if (buffer->count + sizeof(byte) >= buffer->size - 1) {
    size_t new_size = buffer->size * 2;
    if (!new_size) {
      new_size = BUFFER_INIT_SIZE;
    }
    buffer->data = buffer_memory_realloc(buffer->data, new_size);
    buffer->size = new_size;
    memset(&buffer->data[buffer->count], 0, buffer->size - buffer->count);
  }
  buffer->data[buffer->count++] = byte;
}

BUFFER_PUBLICDEF
void buffer_insert(Buffer* buffer, char byte, size_t index) {
  buffer_append(buffer, 0);
  memmove(&buffer->data[index + 1], &buffer->data[index], buffer->count - index + 1);
  buffer->data[index] = byte;
}

BUFFER_PUBLICDEF
void buffer_erase(Buffer* buffer, size_t index) {
  if (index < buffer->count && buffer->count > 0) {
    memmove(&buffer->data[index], &buffer->data[index + 1], (buffer->count - index) - 1);
    buffer->data[--buffer->count] = 0;
    return;
  }
  buffer->data[0] = 0;
  buffer->count = 0;
}

BUFFER_PUBLICDEF
void buffer_free(Buffer* buffer) {
  if (buffer->data) {
    buffer_memory_free(buffer->data);
    buffer->data = NULL;
  }
  buffer->count = 0;
  buffer->size = 0;
}

BUFFER_PUBLICDEF
Buffer buffer_new_from_fd(int fd) {
  size_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  Buffer buffer = buffer_new(file_size);
  if (read(fd, buffer.data, file_size) == (ssize_t)file_size) {
    buffer.count = file_size;
    buffer.size = file_size;
  }
  else {
    buffer_free(&buffer);
  }
  return buffer;
}

#endif // BUFFER_IMPL
