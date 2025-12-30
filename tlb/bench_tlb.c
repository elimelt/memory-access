#include "bench.h"

#include <stdio.h>
#include <time.h>

static BenchResult RunStride(uint64_t *array, size_t n, size_t stride, const char *name) {
  size_t count = n / stride;
  if (count < 1) count = 1;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = 0;
  for (size_t rep = 0; rep < 10; rep++) {
    for (size_t i = 0; i < n; i += stride) {
      sum += array[i];
      Clobber();
    }
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  Escape(&sum);

  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
  size_t total = count * 10;

  return (BenchResult){name, total, ns, (double)ns / total};
}

BenchResult BenchTlbSeq(uint64_t *a, size_t n) {
  return RunStride(a, n, 1, "Stride 8B (sequential)");
}

BenchResult BenchTlb64(uint64_t *a, size_t n) {
  return RunStride(a, n, 8, "Stride 64B (cache line)");
}

BenchResult BenchTlb512(uint64_t *a, size_t n) {
  return RunStride(a, n, 64, "Stride 512B");
}

BenchResult BenchTlbPage(uint64_t *a, size_t n) {
  return RunStride(a, n, 512, "Stride 4KB (1 per page)");
}

BenchResult BenchTlb2Page(uint64_t *a, size_t n) {
  return RunStride(a, n, 1024, "Stride 8KB (skip pages)");
}

