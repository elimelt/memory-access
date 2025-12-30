#include "bench.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void Shuffle(size_t *arr, size_t n) {
  srand(42);
  for (size_t i = n - 1; i > 0; i--) {
    size_t j = rand() % (i + 1);
    size_t tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
  }
}

static BenchResult RunPrefetch(uint64_t *array, size_t n, size_t dist, const char *name) {
  size_t *indices = malloc(n * sizeof(size_t));
  for (size_t i = 0; i < n; i++) indices[i] = i;
  Shuffle(indices, n);

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = 0;
  if (dist == 0) {
    for (size_t i = 0; i < n; i++) {
      sum += array[indices[i]];
      Clobber();
    }
  } else {
    for (size_t i = 0; i < n; i++) {
      if (i + dist < n) __builtin_prefetch(&array[indices[i + dist]], 0, 0);
      sum += array[indices[i]];
      Clobber();
    }
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  Escape(&sum);
  free(indices);

  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
  return (BenchResult){name, n, ns, (double)ns / n};
}

static BenchResult RunSeqPrefetch(uint64_t *array, size_t n, size_t dist, const char *name) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = 0;
  if (dist == 0) {
    for (size_t i = 0; i < n; i++) {
      sum += array[i];
      Clobber();
    }
  } else {
    for (size_t i = 0; i < n; i++) {
      if (i + dist < n) __builtin_prefetch(&array[i + dist], 0, 0);
      sum += array[i];
      Clobber();
    }
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  Escape(&sum);

  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
  return (BenchResult){name, n, ns, (double)ns / n};
}

BenchResult BenchPrefetchNone(uint64_t *a, size_t n) {
  return RunPrefetch(a, n, 0, "Random no prefetch");
}
BenchResult BenchPrefetch8(uint64_t *a, size_t n) {
  return RunPrefetch(a, n, 8, "Random prefetch +8");
}
BenchResult BenchPrefetch32(uint64_t *a, size_t n) {
  return RunPrefetch(a, n, 32, "Random prefetch +32");
}
BenchResult BenchPrefetch128(uint64_t *a, size_t n) {
  return RunPrefetch(a, n, 128, "Random prefetch +128");
}

BenchResult BenchSeqPrefetchNone(uint64_t *a, size_t n) {
  return RunSeqPrefetch(a, n, 0, "Sequential no sw prefetch");
}
BenchResult BenchSeqPrefetch64(uint64_t *a, size_t n) {
  return RunSeqPrefetch(a, n, 64, "Sequential sw prefetch +64");
}

