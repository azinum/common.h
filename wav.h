// wav.h
// quick and dirty wav importer and exporter

// flags:
//  WAV_NO_UNISTD -- supply your own read and write functions

#ifndef _WAV_H
#define _WAV_H

#include <stddef.h> // size_t
#include <stdint.h>
#ifndef WAV_NO_UNISTD
  #include <unistd.h> // read, write
#endif

#ifdef __cplusplus
  #define RESTRICT
  #define WAV_PUBLICDEC extern "C"
  #define WAV_PUBLICDEF extern "C"
#else
  #define RESTRICT restrict
  #define WAV_PUBLICDEC extern
  #define WAV_PUBLICDEF
#endif

#ifndef WAV_MEMORY_MALLOC
  #define WAV_MEMORY_MALLOC malloc
#endif
#ifndef WAV_MEMORY_CALLOC
  #define WAV_MEMORY_CALLOC calloc
#endif
#ifndef WAV_MEMORY_REALLOC
  #define WAV_MEMORY_REALLOC realloc
#endif
#ifndef WAV_MEMORY_FREE
  #define WAV_MEMORY_FREE free
#endif

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

typedef enum Wav_sample_format {
  WAV_FORMAT_INT8 = 0,
  WAV_FORMAT_INT16,
  WAV_FORMAT_FLOAT32,

  MAX_WAV_SAMPLE_FORMAT,
} Wav_sample_format;

typedef enum Wav_error {
  WAV_NO_ERROR,
  WAV_ERROR,
} Wav_error;

const size_t wav_sample_sizes[] = {
  sizeof(i8),
  sizeof(i16),
  sizeof(f32),
};

Wav_error wav_error = WAV_NO_ERROR;

void* (*wav_memory_malloc)(size_t size)           = WAV_MEMORY_MALLOC;
void* (*wav_memory_calloc)(size_t n, size_t size) = WAV_MEMORY_CALLOC;
void* (*wav_memory_realloc)(void* p, size_t size) = WAV_MEMORY_REALLOC;
void  (*wav_memory_free)(void* p)                 = WAV_MEMORY_FREE;

WAV_PUBLICDEC void* wav_import_adv(size_t* sample_count, size_t* channel_count, size_t* sample_rate, Wav_sample_format output_format, i32 fd, ssize_t (*read_func)(i32, void*, size_t));
WAV_PUBLICDEC void* wav_import(size_t* sample_count, size_t* channel_count, size_t* sample_rate, Wav_sample_format output_format, i32 fd);
WAV_PUBLICDEC Wav_error wav_export_from_f32_adv(f32* samples, size_t sample_count, size_t channel_count, size_t sample_rate, i32 fd, ssize_t (*write_func)(i32, const void*, size_t));
WAV_PUBLICDEC Wav_error wav_export_from_f32(f32* samples, size_t sample_count, size_t channel_count, size_t sample_rate, i32 fd);
WAV_PUBLICDEC const char* wav_get_error_string(void);

#endif // _WAV_H

#ifdef WAV_IMPL

#define WAV_MIN_SIZE 44

#define RIFF_ID       {'R', 'I', 'F', 'F'}
#define WAVE_ID       {'W', 'A', 'V', 'E'}
#define DATA_CHUNK_ID {'d', 'a', 't', 'a'}
#define CHUNK_LIST_ID {'L', 'I', 'S', 'T'}
#define FORMAT_ID     {'f', 'm', 't', ' '}

static char riff_id[4]        = RIFF_ID;
static char wave_id[4]        = WAVE_ID;
static char data_chunk_id[4]  = DATA_CHUNK_ID;
static char chunk_list_id[4]  = CHUNK_LIST_ID;
static char format_id[4]      = FORMAT_ID;

typedef struct Wave_header {
  char riff_id[4];
  i32 size;
  char wave_id[4];
} __attribute__((packed)) Wave_header;

typedef struct Wave_format {
  char format_id[4];
  i32 size;
  i16 type;
  i16 channel_count;
  i32 sample_rate;
  i32 data_rate;
  i16 data_block_size;
  i16 bits_per_sample;
} __attribute__((packed)) Wave_format;

typedef struct Wave_chunk {
  char chunk_id[4];
  i32 size;
} __attribute__((packed)) Wave_chunk;

#define MAX_WAV_ERROR_STRING_LEN 512
static char wav_error_string[MAX_WAV_ERROR_STRING_LEN] = {0};

#define WAV_SET_ERROR(message) wav_set_error(__FUNCTION__, message)

static size_t wav_djb2_hash(char* data, size_t n);
static void wav_set_error(const char* function_name, const char* err_string);

size_t wav_djb2_hash(char* data, size_t n) {
  size_t h = 5381;
  for (size_t i = 0; i < n; ++i) {
    h = ((h << 5) + h) + data[i];
  }
  return h;
}

void wav_set_error(const char* function_name, const char* err_string) {
  wav_error = WAV_ERROR;
  snprintf(wav_error_string, MAX_WAV_ERROR_STRING_LEN, "%s: %s", function_name, err_string);
}

WAV_PUBLICDEF
void* wav_import_adv(size_t* sample_count, size_t* channel_count, size_t* sample_rate, Wav_sample_format output_format, i32 fd, ssize_t (*read_func)(i32, void*, size_t)) {
  Wave_header header = {0};
  Wave_format format = {0};
  Wave_chunk chunk = {0};

  if (output_format < 0 || output_format >= MAX_WAV_SAMPLE_FORMAT) {
    output_format = WAV_FORMAT_INT16;
  }

  i32 bytes_read = 0;
  size_t data_chunk_size = 0;
  i16* in_samples = NULL;
  void* out_samples = NULL;
  size_t size_per_sample =  wav_sample_sizes[output_format];

  off_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  if (file_size < WAV_MIN_SIZE) {
    WAV_SET_ERROR("file is too small");
    return NULL;
  }

  bytes_read = read_func(fd, &header, sizeof(header));
  if (bytes_read != sizeof(header)) {
    WAV_SET_ERROR("could not read header");
    return NULL;
  }
  bytes_read = read_func(fd, &format, sizeof(format));
  if (bytes_read != sizeof(format)) {
    WAV_SET_ERROR("could not read format");
    return NULL;
  }
  bytes_read = read_func(fd, &chunk, sizeof(chunk));
  if (bytes_read != sizeof(chunk)) {
    WAV_SET_ERROR("could not read data chunk");
    return NULL;
  }
  if (wav_djb2_hash(chunk.chunk_id, sizeof(chunk.chunk_id)) == wav_djb2_hash("LIST", 4)) {
    // skip the LIST chunk
    lseek(fd, chunk.size, SEEK_CUR);
    // try to read the data chunk
    bytes_read = read_func(fd, &chunk, sizeof(chunk));
    if (bytes_read != sizeof(chunk)) {
      WAV_SET_ERROR("could not read data chunk");
      return NULL;
    }
  }

  *sample_count = format.channel_count * (chunk.size / format.data_block_size);
  *channel_count = format.channel_count;
  data_chunk_size = chunk.size;

  in_samples = (i16*)wav_memory_malloc(data_chunk_size);
  if (!in_samples) {
    WAV_SET_ERROR("could not allocate memory for data chunk");
    return NULL;
  }

  bytes_read = read_func(fd, in_samples, data_chunk_size);
  if (bytes_read != data_chunk_size) {
    wav_memory_free(in_samples);
    WAV_SET_ERROR("could not read data chunk");
    return NULL;
  }

  out_samples = wav_memory_malloc(*sample_count * size_per_sample);

  if (!out_samples) {
    wav_memory_free(in_samples);
    WAV_SET_ERROR("could not allocate memory for output samples");
    return NULL;
  }

  void* it = out_samples;
  for (size_t i = 0; i < *sample_count; ++i, it += size_per_sample) {
    switch (output_format) {
      case WAV_FORMAT_INT8: {
        *(char*)it = in_samples[i] / INT8_MAX;
        break;
      }
      case WAV_FORMAT_INT16: {
        *(i16*)it = in_samples[i];
        break;
      }
      case WAV_FORMAT_FLOAT32: {
        *(f32*)it = in_samples[i] / (f32)INT16_MAX;
        break;
      }
      default: {
        ASSERT(0);
        break;
      }
    }
  }

  return out_samples;
}

WAV_PUBLICDEF
void* wav_import(size_t* sample_count, size_t* channel_count, size_t* sample_rate, Wav_sample_format output_format, i32 fd) {
  return wav_import_adv(sample_count, channel_count, sample_rate, output_format, fd, read);
}

WAV_PUBLICDEF
Wav_error wav_export_from_f32_adv(f32* samples, size_t sample_count, size_t channel_count, size_t sample_rate, i32 fd, ssize_t (*write_func)(i32, const void*, size_t)) {
  i16 bits_per_sample = 16;
  i16 data_block_size = bits_per_sample / 8;
  i32 data_chunk_size = (sample_count * channel_count) * data_block_size;
  i32 total_size = WAV_MIN_SIZE + data_chunk_size - sizeof(Wave_chunk);

  Wave_header header = (Wave_header) {
    .riff_id = RIFF_ID,
    .size = total_size,
    .wave_id = WAVE_ID,
  };

  Wave_format format = (Wave_format) {
    .format_id = FORMAT_ID,
    .size = bits_per_sample,
    .type = 1, // pcm
    .channel_count = channel_count,
    .sample_rate = sample_rate,
    .data_rate = (sample_rate * channel_count * bits_per_sample) / 8,
    .data_block_size = data_block_size,
    .bits_per_sample = bits_per_sample,
  };

  Wave_chunk chunk = (Wave_chunk) {
    .chunk_id = DATA_CHUNK_ID,
    .size = data_chunk_size,
  };

  // no validation, just chug along

  write_func(fd, &header, sizeof(header));
  write_func(fd, &format, sizeof(format));
  write_func(fd, &chunk, sizeof(chunk));

  i16* it16 = (i16*)samples;
  f32* it = samples;
  for (size_t i = 0; i < sample_count; ++i, ++it, ++it16) {
    *it16 = (i16)(*it * INT16_MAX);
  }

  write_func(fd, samples, sizeof(i16) * sample_count);

  return WAV_NO_ERROR;
}

WAV_PUBLICDEF
Wav_error wav_export_from_f32(f32* samples, size_t sample_count, size_t channel_count, size_t sample_rate, i32 fd) {
  return wav_export_from_f32_adv(samples, sample_count, channel_count, sample_rate, fd, write);
}

WAV_PUBLICDEF
const char* wav_get_error_string(void) {
  return wav_error_string;
}

#endif // WAV_IMPL
