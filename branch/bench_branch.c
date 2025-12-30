#include "bench.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int cmp(const void *a, const void *b) {
  return (*(uint64_t *)a > *(uint64_t *)b) - (*(uint64_t *)a < *(uint64_t *)b);
}

BenchResult BenchBranchSorted(uint64_t *array, size_t n) {
  qsort(array, n, sizeof(uint64_t), cmp);
  uint64_t threshold = array[n / 2];

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = 0;
  for (size_t i = 0; i < n; i++) {
    if (array[i] < threshold)
      sum += array[i];
    Clobber();
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  Escape(&sum);

  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
  return (BenchResult){"Branch sorted (predictable)", n, ns, (double)ns / n};
}

BenchResult BenchBranchRandom(uint64_t *array, size_t n) {
  srand(42);
  for (size_t i = 0; i < n; i++)
    array[i] = rand();

  uint64_t threshold = RAND_MAX / 2;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = 0;
  for (size_t i = 0; i < n; i++) {
    if (array[i] < threshold)
      sum += array[i];
    Clobber();
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  Escape(&sum);

  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
  return (BenchResult){"Branch random (unpredictable)", n, ns, (double)ns / n};
}

BenchResult BenchBranchless(uint64_t *array, size_t n) {
  srand(42);
  for (size_t i = 0; i < n; i++)
    array[i] = rand();

  uint64_t threshold = RAND_MAX / 2;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = 0;
  for (size_t i = 0; i < n; i++) {
    uint64_t mask = -(array[i] < threshold);
    sum += array[i] & mask;
    Clobber();
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  Escape(&sum);

  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
  return (BenchResult){"Branchless (mask)", n, ns, (double)ns / n};
}

