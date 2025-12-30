#include "bench.h"

#include <stdio.h>
#include <time.h>

BenchResult BenchStoreFwdSame(uint64_t *array, size_t n) {
  uint8_t *bytes = (uint8_t *)array;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = 0;
  for (size_t i = 0; i < n; i++) {
    *(uint64_t *)(bytes) = i;
    sum += *(uint64_t *)(bytes);
    Clobber();
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  Escape(&sum);

  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
  return (BenchResult){"Store-load aligned (fast forward)", n, ns, (double)ns / n};
}

BenchResult BenchStoreFwdDiff(uint64_t *array, size_t n) {
  uint8_t *bytes = (uint8_t *)array;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = 0;
  for (size_t i = 0; i < n; i++) {
    *(uint64_t *)(bytes + 1) = i;
    sum += *(uint64_t *)(bytes);
    Clobber();
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  Escape(&sum);

  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
  return (BenchResult){"Store-load overlap (stall)", n, ns, (double)ns / n};
}

BenchResult BenchStoreFwdNone(uint64_t *array, size_t n) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = 0;
  for (size_t i = 0; i < n; i++) {
    array[0] = i;
    sum += array[64];
    Clobber();
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  Escape(&sum);

  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
  return (BenchResult){"Store-load independent (no dep)", n, ns, (double)ns / n};
}

