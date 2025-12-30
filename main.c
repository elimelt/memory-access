#include "bench.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const BenchEntry kBenchmarks[] = {
    {"seq", "Sequential access", BenchSequential},
    {"ran", "Random access (parallel loads)", BenchRandom},
    {"chase", "Pointer chasing (serial loads)", BenchPointerChase},
    {"red_naive", "Reduction naive (1 accumulator)", BenchReductionNaive},
    {"red_ilp", "Reduction ILP (8 accumulators)", BenchReductionILP},
    {"red_simd", "Reduction SIMD (AVX2/SSE2)", BenchReductionSimd},
    {"red_thread", "Reduction threaded (8 threads)", BenchReductionThread},
    {"red_ilp_simd", "Reduction ILP+SIMD combined", BenchReductionILPSimd},
    {"red_all", "Reduction all (threads+ILP+SIMD)", BenchReductionAll},
    {"red_opt", "Reduction optimized (compiler free)", BenchReductionOpt},
    {"mlp1", "Chase 1 chain (MLP=1)", BenchChase1},
    {"mlp2", "Chase 2 chains (MLP=2)", BenchChase2},
    {"mlp4", "Chase 4 chains (MLP=4)", BenchChase4},
    {"mlp8", "Chase 8 chains (MLP=8)", BenchChase8},
    {"mlp16", "Chase 16 chains (MLP=16)", BenchChase16},
    {"pf_none", "Random no prefetch", BenchPrefetchNone},
    {"pf_8", "Random prefetch +8", BenchPrefetch8},
    {"pf_32", "Random prefetch +32", BenchPrefetch32},
    {"pf_128", "Random prefetch +128", BenchPrefetch128},
    {"pf_seq", "Sequential no sw prefetch", BenchSeqPrefetchNone},
    {"pf_seq64", "Sequential sw prefetch +64", BenchSeqPrefetch64},
    {"fs_bad", "False sharing (packed)", BenchFalseSharing},
    {"fs_good", "No false sharing (padded)", BenchNoFalseSharing},
    {"tlb_seq", "Stride 8B (sequential)", BenchTlbSeq},
    {"tlb_64", "Stride 64B (cache line)", BenchTlb64},
    {"tlb_512", "Stride 512B", BenchTlb512},
    {"tlb_4k", "Stride 4KB (1 per page)", BenchTlbPage},
    {"tlb_8k", "Stride 8KB (skip pages)", BenchTlb2Page},
    {"br_sort", "Branch sorted (predictable)", BenchBranchSorted},
    {"br_rand", "Branch random (unpredictable)", BenchBranchRandom},
    {"br_less", "Branchless (mask)", BenchBranchless},
    {"bw_1", "Bandwidth 1 thread", BenchBw1},
    {"bw_2", "Bandwidth 2 threads", BenchBw2},
    {"bw_4", "Bandwidth 4 threads", BenchBw4},
    {"bw_8", "Bandwidth 8 threads", BenchBw8},
    {"sf_fwd", "Store-load aligned (forwarding)", BenchStoreFwdSame},
    {"sf_stall", "Store-load overlap (stall)", BenchStoreFwdDiff},
    {"sf_indep", "Store-load independent (no dep)", BenchStoreFwdNone},
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
  fprintf(stderr, "  %-12s - %s\n", "all", "Run all benchmarks");
  for (size_t i = 0; i < kNumBenchmarks; i++) {
    fprintf(stderr, "  %-12s - %s\n", kBenchmarks[i].cli_name,
            kBenchmarks[i].description);
  }
  fprintf(stderr, "\nOptional:\n");
  fprintf(stderr, "  array_size_mb - Size of array in MB (default: 128)\n");
  fprintf(stderr, "\nExample: %s seq 256\n", prog_name);
  fprintf(stderr, "         %s all 64\n", prog_name);
}

static void PrintResult(const BenchResult *result) {
  printf("\n=== %s ===\n", result->name);
  printf("Iterations:     %zu\n", result->iterations);
  printf("Total time:     %.2f ms\n", result->total_ns / 1e6);
  printf("Time per access: %.2f ns\n", result->ns_per_access);
  printf("\n");
}

static int RunAllBenchmarks(uint64_t *array, size_t n) {
  printf("\n========================================\n");
  printf("Running all %zu benchmarks...\n", kNumBenchmarks);
  printf("========================================\n");

  for (size_t i = 0; i < kNumBenchmarks; i++) {
    BenchResult result = kBenchmarks[i].func(array, n);
    PrintResult(&result);
  }

  printf("========================================\n");
  printf("All benchmarks complete.\n");
  printf("========================================\n");
  return 0;
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

  int run_all = (strcmp(bench_type, "all") == 0);

  if (!run_all) {
    const BenchEntry *bench = FindBenchmark(bench_type);
    if (!bench) {
      fprintf(stderr, "Error: Unknown benchmark type '%s'\n", bench_type);
      PrintUsage(argv[0]);
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

  if (run_all) {
    RunAllBenchmarks(array, n);
  } else {
    const BenchEntry *bench = FindBenchmark(bench_type);
    BenchResult result = bench->func(array, n);
    PrintResult(&result);
  }

  free(array);
  return 0;
}
