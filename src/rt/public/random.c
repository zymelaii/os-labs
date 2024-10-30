#include <random.h>

static size_t __RANDOM_SEED = 2;

static void lcg_random() {
  __RANDOM_SEED = (1664525 * __RANDOM_SEED + 1013904223) & ~0ul;
}

void srand(size_t seed) {
  __RANDOM_SEED = seed;
  lcg_random();
}

size_t rand() {
  lcg_random();
  return __RANDOM_SEED;
}
