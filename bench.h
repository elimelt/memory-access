#ifndef BENCH_H_
#define BENCH_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
  const char *name;
  size_t iterations;
  uint64_t total_ns;
  double ns_per_access;
} BenchResult;

static inline void Escape(void *p) {
  __asm__ volatile("" : : "g"(p) : "memory");
}

static inline void Clobber(void) {
  __asm__ volatile("" : : : "memory");
}

// Benchmark function signature
typedef BenchResult (*BenchFunc)(uint64_t *array, size_t n);

// Benchmark registry entry
typedef struct {
  const char *cli_name;    // Command line name (e.g., "seq")
  const char *description; // Help text description
  BenchFunc func;          // Function pointer
} BenchEntry;

// Memory access benchmarks
BenchResult BenchSequential(uint64_t *array, size_t n);
BenchResult BenchRandom(uint64_t *array, size_t n);
BenchResult BenchPointerChase(uint64_t *array, size_t n);

// Reduction benchmarks (ILP demonstration)
BenchResult BenchReductionNaive(uint64_t *array, size_t n);
BenchResult BenchReductionILP(uint64_t *array, size_t n);
BenchResult BenchReductionSimd(uint64_t *array, size_t n);
BenchResult BenchReductionThread(uint64_t *array, size_t n);
BenchResult BenchReductionILPSimd(uint64_t *array, size_t n);
BenchResult BenchReductionAll(uint64_t *array, size_t n);

#endif
