// test_common.h

#ifndef _TEST_COMMON_H
#define _TEST_COMMON_H

#ifdef VERBOSE
  #define verbose_printf(...) printf(__VA_ARGS__)
  #define verbose_dprintf(...) dprintf(__VA_ARGS__)
#else
  #define verbose_printf(...)
  #define verbose_dprintf(...)
#endif

#endif // _TEST_COMMON_H
