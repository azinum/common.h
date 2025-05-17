// log.h

#ifndef _LOG_H
#define _LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LOG_CUSTOM_TAGS

typedef enum {
  LOG_TAG_NONE,
  LOG_TAG_INFO,
  LOG_TAG_WARN,
  LOG_TAG_ERROR,
  LOG_TAG_SUCCESS,
  LOG_TAG_DEBUG,

  MAX_LOG_TAG,
} Log_tag;

typedef enum {
  LOG_COLOR_NONE = 0,
  LOG_COLOR_RESET,
  LOG_COLOR_BLUE,
  LOG_COLOR_RED,
  LOG_COLOR_GREEN,
  LOG_COLOR_BOLD_WHITE,
  LOG_COLOR_YELLOW,
  LOG_COLOR_GRAY,

  MAX_COLOR,
} Log_color;

const char* log_tags[MAX_LOG_TAG] = {
  [LOG_TAG_NONE]    = "",
  [LOG_TAG_INFO]    = "info",
  [LOG_TAG_WARN]    = "warning",
  [LOG_TAG_ERROR]   = "error",
  [LOG_TAG_SUCCESS] = "success",
  [LOG_TAG_DEBUG]   = "debug",
};

Log_color log_tag_colors[MAX_LOG_TAG] = {
  [LOG_TAG_NONE]    = LOG_COLOR_NONE,
  [LOG_TAG_INFO]    = LOG_COLOR_BLUE,
  [LOG_TAG_WARN]    = LOG_COLOR_YELLOW,
  [LOG_TAG_ERROR]   = LOG_COLOR_RED,
  [LOG_TAG_SUCCESS] = LOG_COLOR_GREEN,
  [LOG_TAG_DEBUG]   = LOG_COLOR_GRAY,
};

#endif

void log_init(bool use_colors);
void log_print(i32 fd, Log_tag tag, const char* fmt, ...);
void log_print_tag(i32 fd, const char* tag, Log_color tag_color);

#ifdef __cplusplus
}
#endif

#endif // _LOG_H

#ifdef LOG_IMPL

struct {
  bool use_colors;
} log_state = {
  .use_colors = false,
};

#define USE_COLOR_FD(FD, LOG_COLOR, ...) { color_begin(LOG_COLOR); __VA_ARGS__; color_end(); }
#define USE_COLOR(LOG_COLOR, ...) USE_COLOR_FD(STDOUT_FILENO, LOG_COLOR, __VA_ARGS__)

static const char* color_str[MAX_COLOR] = {
  [LOG_COLOR_NONE]       = "",
  [LOG_COLOR_RESET]      = "\033[0;00m",
  [LOG_COLOR_BLUE]       = "\033[0;34m",
  [LOG_COLOR_RED]        = "\033[0;31m",
  [LOG_COLOR_GREEN]      = "\033[0;32m",
  [LOG_COLOR_BOLD_WHITE] = "\033[1;37m",
  [LOG_COLOR_YELLOW]     = "\033[1;33m",
  [LOG_COLOR_GRAY]       = "\033[0;90m",
};

static void log_color_begin(Log_color color);
static void log_color_end(void);

void log_color_begin(Log_color color) {
#ifndef NO_COLORS
  if (log_state.use_colors && (color >= 0) && (color < MAX_COLOR)) {
    dprintf(STDOUT_FILENO, "%s", color_str[color]);
  }
#endif
}

void log_color_end(void) {
#ifndef NO_COLORS
  if (log_state.use_colors) {
    dprintf(STDOUT_FILENO, "%s", color_str[LOG_COLOR_RESET]);
  }
#endif
}

void log_init(bool use_colors) {
#ifndef NO_COLORS
  if (log_state.use_colors) {
    use_colors = enable_vt100_mode();
  }
  log_state.use_colors = use_colors;
#else
  log_state.use_colors = false;
#endif
}

void log_print(i32 fd, Log_tag tag, const char* fmt, ...) {
  ASSERT(tag < MAX_LOG_TAG && "invalid tag");
  if (tag != LOG_TAG_NONE) {
    log_print_tag(fd, log_tags[tag], log_tag_colors[tag]);
  }
  va_list argp;
  va_start(argp, fmt);
  // TODO: create a fdprintf that will convert a file descriptor to FILE* and pass that to fprintf
  // hack!
#ifdef TARGET_WINDOWS
  vprintf(fmt, argp);
#else
  vdprintf(fd, fmt, argp);
#endif
  va_end(argp);
}

void log_print_tag(i32 fd, const char* tag, Log_color tag_color) {
  ASSERT(tag != NULL);
  ASSERT(tag_color < MAX_COLOR);

  log_color_begin(tag_color);
#ifdef TARGET_WINDOWS
  printf("[%s]: ", tag);
#else
 dprintf(fd, "[%s]: ", tag);
#endif
  log_color_end();
}

#undef LOG_IMPL
#endif // LOG_IMPL
