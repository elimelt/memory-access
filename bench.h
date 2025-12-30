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
BenchResult BenchReductionOpt(uint64_t *array, size_t n);

// Multi-chain pointer chase (MLP demonstration)
BenchResult BenchChase1(uint64_t *array, size_t n);
BenchResult BenchChase2(uint64_t *array, size_t n);
BenchResult BenchChase4(uint64_t *array, size_t n);
BenchResult BenchChase8(uint64_t *array, size_t n);
BenchResult BenchChase16(uint64_t *array, size_t n);

// Software prefetching
BenchResult BenchPrefetchNone(uint64_t *array, size_t n);
BenchResult BenchPrefetch8(uint64_t *array, size_t n);
BenchResult BenchPrefetch32(uint64_t *array, size_t n);
BenchResult BenchPrefetch128(uint64_t *array, size_t n);
BenchResult BenchSeqPrefetchNone(uint64_t *array, size_t n);
BenchResult BenchSeqPrefetch64(uint64_t *array, size_t n);

// False sharing
BenchResult BenchFalseSharing(uint64_t *array, size_t n);
BenchResult BenchNoFalseSharing(uint64_t *array, size_t n);

// TLB
BenchResult BenchTlbSeq(uint64_t *array, size_t n);
BenchResult BenchTlb64(uint64_t *array, size_t n);
BenchResult BenchTlb512(uint64_t *array, size_t n);
BenchResult BenchTlbPage(uint64_t *array, size_t n);
BenchResult BenchTlb2Page(uint64_t *array, size_t n);

// Branch prediction
BenchResult BenchBranchSorted(uint64_t *array, size_t n);
BenchResult BenchBranchRandom(uint64_t *array, size_t n);
BenchResult BenchBranchless(uint64_t *array, size_t n);

// Bandwidth
BenchResult BenchBw1(uint64_t *array, size_t n);
BenchResult BenchBw2(uint64_t *array, size_t n);
BenchResult BenchBw4(uint64_t *array, size_t n);
BenchResult BenchBw8(uint64_t *array, size_t n);

// Store-to-load forwarding
BenchResult BenchStoreFwdSame(uint64_t *array, size_t n);
BenchResult BenchStoreFwdDiff(uint64_t *array, size_t n);
BenchResult BenchStoreFwdNone(uint64_t *array, size_t n);

#endif
