#include "bench.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void PrintUsage(const char *prog_name) {
  fprintf(stderr, "Usage: %s <benchmark_type> [array_size_mb]\n", prog_name);
  fprintf(stderr, "Benchmark types:\n");
  fprintf(stderr, "  seq    - Sequential access\n");
  fprintf(stderr, "  ran    - Random access (parallel loads)\n");
  fprintf(stderr, "  chase  - Pointer chasing (serial loads)\n");
  fprintf(stderr, "\nOptional:\n");
  fprintf(stderr, "  array_size_mb - Size of array in MB (default: 128)\n");
  fprintf(stderr, "\nExample: %s seq 256\n", prog_name);
}

static void PrintResult(const BenchResult *result) {
  printf("\n=== %s ===\n", result->name);
  printf("Iterations:     %zu\n", result->iterations);
  printf("Total time:     %.2f ms\n", result->total_ns / 1e6);
  printf("Time per access: %.2f ns\n", result->ns_per_access);
  printf("\n");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    PrintUsage(argv[0]);
    return 1;
  }

  const char *bench_type = argv[1];
  size_t size_mb = 128;

  if (argc >= 3) {
    size_mb = atoi(argv[2]);
    if (size_mb == 0 || size_mb > 4096) {
      fprintf(stderr, "Error: Array size must be between 1 and 4096 MB\n");
      return 1;
    }
  }

  size_t n = (size_mb * 1024 * 1024) / sizeof(uint64_t);
  printf("Allocating %zu MB array...\n", size_mb);

  uint64_t *array = malloc(n * sizeof(uint64_t));
  if (!array) {
    fprintf(stderr, "Failed to allocate %zu MB\n", size_mb);
    return 1;
  }

  printf("Initializing array...\n");
  for (size_t i = 0; i < n; i++) {
    array[i] = i;
  }

  BenchResult result = {0};

  if (strcmp(bench_type, "seq") == 0) {
    result = BenchSequential(array, n);
  } else if (strcmp(bench_type, "ran") == 0) {
    result = BenchRandom(array, n);
  } else if (strcmp(bench_type, "chase") == 0) {
    result = BenchPointerChase(array, n);
  } else {
    fprintf(stderr, "Error: Unknown benchmark type '%s'\n", bench_type);
    PrintUsage(argv[0]);
    free(array);
    return 1;
  }

  PrintResult(&result);
  free(array);
  return 0;
}
