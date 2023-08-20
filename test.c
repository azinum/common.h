// test.c

#define STB_SPRINTF_IMPLEMENTATION
#define USE_STB_SPRINTF
#include "stb_sprintf.h"

#define COMMON_IMPLEMENTATION
#include "common.h"

int add(int a, int b);

int main(void) {
  printf("Hello, World! %d\n", add(2, 3));
  return 0;
}

int add(int a, int b) {
  return a + b;
}
