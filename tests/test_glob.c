// test_glob.c

#include "test_common.h"

#define GLOB_IMPL
#include "glob.h"

int test(void);

int main(void) {
  return test();
}

int test(void) {
  if (!glob("*.txt", "hello.txt")) return 1;
  if (glob("*.txt", "h")) return 1;
  if (!glob("*.*", "hello.txt")) return 1;
  if (!glob("hello.*", "hello.txt")) return 1;
  if (glob("hello.*", "hellq.txt")) return 1;
  if (!glob("hello", "hello")) return 1;
  if (!glob("he??o", "hello")) return 1;
  if (!glob("he??o", "he11o")) return 1;
  return 0;
}

