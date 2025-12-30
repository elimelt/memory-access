#include "bench.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void BuildChain(uint64_t *array, size_t n, size_t *start_indices, size_t num_chains) {
  size_t *indices = malloc(n * sizeof(size_t));
  for (size_t i = 0; i < n; i++) indices[i] = i;

  srand(42);
  for (size_t i = n - 1; i > 0; i--) {
    size_t j = rand() % (i + 1);
    size_t tmp = indices[i];
    indices[i] = indices[j];
    indices[j] = tmp;
  }

  for (size_t i = 0; i < n - 1; i++) {
    array[indices[i]] = indices[i + 1];
  }
  array[indices[n - 1]] = indices[0];

  size_t chunk = n / num_chains;
  for (size_t c = 0; c < num_chains; c++) {
    start_indices[c] = indices[c * chunk];
  }
  free(indices);
}

static BenchResult RunChase(uint64_t *array, size_t n, size_t num_chains, const char *name) {
  size_t starts[16];
  BuildChain(array, n, starts, num_chains);

  size_t idx[16];
  for (size_t c = 0; c < num_chains; c++) idx[c] = starts[c];

  struct timespec start, end;
  size_t iters = n / num_chains;

  clock_gettime(CLOCK_MONOTONIC, &start);

  switch (num_chains) {
    case 1:
      for (size_t i = 0; i < iters; i++) {
        idx[0] = array[idx[0]];
        Clobber();
      }
      break;
    case 2:
      for (size_t i = 0; i < iters; i++) {
        idx[0] = array[idx[0]];
        idx[1] = array[idx[1]];
        Clobber();
      }
      break;
    case 4:
      for (size_t i = 0; i < iters; i++) {
        idx[0] = array[idx[0]]; idx[1] = array[idx[1]];
        idx[2] = array[idx[2]]; idx[3] = array[idx[3]];
        Clobber();
      }
      break;
    case 8:
      for (size_t i = 0; i < iters; i++) {
        idx[0] = array[idx[0]]; idx[1] = array[idx[1]];
        idx[2] = array[idx[2]]; idx[3] = array[idx[3]];
        idx[4] = array[idx[4]]; idx[5] = array[idx[5]];
        idx[6] = array[idx[6]]; idx[7] = array[idx[7]];
        Clobber();
      }
      break;
    case 16:
      for (size_t i = 0; i < iters; i++) {
        idx[0] = array[idx[0]]; idx[1] = array[idx[1]];
        idx[2] = array[idx[2]]; idx[3] = array[idx[3]];
        idx[4] = array[idx[4]]; idx[5] = array[idx[5]];
        idx[6] = array[idx[6]]; idx[7] = array[idx[7]];
        idx[8] = array[idx[8]]; idx[9] = array[idx[9]];
        idx[10] = array[idx[10]]; idx[11] = array[idx[11]];
        idx[12] = array[idx[12]]; idx[13] = array[idx[13]];
        idx[14] = array[idx[14]]; idx[15] = array[idx[15]];
        Clobber();
      }
      break;
  }

  clock_gettime(CLOCK_MONOTONIC, &end);

  for (size_t c = 0; c < num_chains; c++) Escape(&idx[c]);

  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
  size_t total_accesses = iters * num_chains;

  return (BenchResult){name, total_accesses, ns, (double)ns / total_accesses};
}

BenchResult BenchChase1(uint64_t *a, size_t n) { return RunChase(a, n, 1, "Chase 1 chain (MLP=1)"); }
BenchResult BenchChase2(uint64_t *a, size_t n) { return RunChase(a, n, 2, "Chase 2 chains (MLP=2)"); }
BenchResult BenchChase4(uint64_t *a, size_t n) { return RunChase(a, n, 4, "Chase 4 chains (MLP=4)"); }
BenchResult BenchChase8(uint64_t *a, size_t n) { return RunChase(a, n, 8, "Chase 8 chains (MLP=8)"); }
BenchResult BenchChase16(uint64_t *a, size_t n) { return RunChase(a, n, 16, "Chase 16 chains (MLP=16)"); }

