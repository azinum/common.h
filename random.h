// random.h

#ifndef _RANDOM_H
#define _RANDOM_H

#ifdef __cplusplus
  #define RESTRICT
  #define RANDOM_PUBLICDEC extern "C"
  #define RANDOM_PUBLICDEF extern "C"
#else
  #define RESTRICT restrict
  #define RANDOM_PUBLICDEC extern
  #define RANDOM_PUBLICDEF
#endif

#ifndef Random
  typedef size_t Random;
#endif

RANDOM_PUBLICDEC void random_init(Random seed);
RANDOM_PUBLICDEC Random random_get_current_seed(void);
RANDOM_PUBLICDEC Random random_lc(void);
RANDOM_PUBLICDEC Random random_xor_shift(void);
RANDOM_PUBLICDEC Random random_number(void);
RANDOM_PUBLICDEC f32 random_f32(void);

#endif // _RANDOM_H

#ifdef RANDOM_IMPLEMENTATION

static Random current_seed = 2147483647;

RANDOM_PUBLICDEF
void random_init(Random seed) {
  current_seed = seed;
}

RANDOM_PUBLICDEF
Random random_get_current_seed(void) {
  return current_seed;
}

RANDOM_PUBLICDEF
Random random_lc(void) {
  const Random a = 16807;
  const Random multiplier = 2147483647;
  const Random increment = 13;
  return (current_seed = (current_seed * a + increment) % multiplier);
}

RANDOM_PUBLICDEF
Random random_xor_shift(void) {
  current_seed ^= current_seed << 13;
  current_seed ^= current_seed >> 17;
  current_seed ^= current_seed << 5;
  return current_seed;
}

RANDOM_PUBLICDEF
Random random_number(void) {
  return random_lc();
}

RANDOM_PUBLICDEF
f32 random_f32(void) {
  return (i32)random_number() / (f32)INT32_MAX;
}

#endif // RANDOM_IMPLEMENTATION
