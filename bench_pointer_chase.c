#include "bench.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

BenchResult BenchPointerChase(uint64_t *array, size_t n) {
  size_t *indices = malloc(n * sizeof(size_t));
  if (!indices) {
    fprintf(stderr, "Failed to allocate indices\n");
    BenchResult error = {0};
    return error;
  }

  for (size_t i = 0; i < n; i++) {
    indices[i] = i;
  }

  srand(42);
  for (size_t i = n - 1; i > 0; i--) {
    size_t j = rand() % (i + 1);
    size_t temp = indices[i];
    indices[i] = indices[j];
    indices[j] = temp;
  }

  for (size_t i = 0; i < n - 1; i++) {
    array[indices[i]] = indices[i + 1];
  }
  array[indices[n - 1]] = indices[0];

  free(indices);

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  size_t index = 0;
  for (size_t i = 0; i < n; i++) {
    index = array[index];
    Clobber();
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL +
                (end.tv_nsec - start.tv_nsec);

  Escape(&index);

  BenchResult result = {
      .name = "Pointer Chase (Serial DRAM Latency)",
      .iterations = n,
      .total_ns = ns,
      .ns_per_access = (double)ns / n
  };

  return result;
}
