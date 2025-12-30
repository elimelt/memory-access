#include "bench.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Benchmark registry - add new benchmarks here
static const BenchEntry kBenchmarks[] = {
    // Memory access patterns
    {"seq", "Sequential access", BenchSequential},
    {"ran", "Random access (parallel loads)", BenchRandom},
    {"chase", "Pointer chasing (serial loads)", BenchPointerChase},
    // Reduction benchmarks (ILP demonstration)
    {"red_naive", "Reduction naive (1 accumulator)", BenchReductionNaive},
    {"red_ilp", "Reduction ILP (8 accumulators)", BenchReductionILP},
    {"red_simd", "Reduction SIMD (AVX2/SSE2)", BenchReductionSimd},
    {"red_thread", "Reduction threaded (8 threads)", BenchReductionThread},
    {"red_ilp_simd", "Reduction ILP+SIMD combined", BenchReductionILPSimd},
    {"red_all", "Reduction all (threads+ILP+SIMD)", BenchReductionAll},
};

static const size_t kNumBenchmarks = sizeof(kBenchmarks) / sizeof(kBenchmarks[0]);

static const BenchEntry *FindBenchmark(const char *name) {
  for (size_t i = 0; i < kNumBenchmarks; i++) {
    if (strcmp(name, kBenchmarks[i].cli_name) == 0) {
      return &kBenchmarks[i];
    }
  }
  return NULL;
}

static void PrintUsage(const char *prog_name) {
  fprintf(stderr, "Usage: %s <benchmark_type> [array_size_mb]\n", prog_name);
  fprintf(stderr, "\nBenchmark types:\n");
  for (size_t i = 0; i < kNumBenchmarks; i++) {
    fprintf(stderr, "  %-8s - %s\n", kBenchmarks[i].cli_name,
            kBenchmarks[i].description);
  }
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

  const BenchEntry *bench = FindBenchmark(bench_type);
  if (!bench) {
    fprintf(stderr, "Error: Unknown benchmark type '%s'\n", bench_type);
    PrintUsage(argv[0]);
    return 1;
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

  BenchResult result = bench->func(array, n);
  PrintResult(&result);

  free(array);
  return 0;
}
