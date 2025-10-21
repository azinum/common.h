// glob.h

#ifndef _GLOB_H
#define _GLOB_H

#include <stdbool.h>

// * = match any string
// ? = match any character
bool glob(const char* pattern, const char* path);

#endif // _GLOB_H

#ifdef GLOB_IMPL

bool glob(const char* pattern, const char* path) {
  for (;;) {
    if (*pattern == '*') {
      if (*(pattern+1) == *(path+1)) {
        pattern += 1;
      }
      path += 1;
    }
    else if (*pattern == '?') {
      pattern += 1;
      path += 1;
    }
    else if (*pattern == *path) {
      if (*pattern == 0) {
        return true;
      }
      path += 1;
      pattern += 1;
    }
    else {
      return false;
    }
  }
  return false;
}

#endif // GLOB_IMPL
