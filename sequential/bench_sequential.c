#include "bench.h"

#include <stdio.h>
#include <time.h>

BenchResult BenchSequential(uint64_t *array, size_t n) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = 0;
  for (size_t i = 0; i < n; i++) {
    sum += array[i];
    Clobber();
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL +
                (end.tv_nsec - start.tv_nsec);

  Escape(&sum);

  BenchResult result = {
      .name = "Sequential Access",
      .iterations = n,
      .total_ns = ns,
      .ns_per_access = (double)ns / n
  };

  return result;
}
