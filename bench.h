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

BenchResult BenchSequential(uint64_t *array, size_t n);
BenchResult BenchRandom(uint64_t *array, size_t n);
BenchResult BenchPointerChase(uint64_t *array, size_t n);

#endif
