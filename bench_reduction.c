#include "bench.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __AVX2__
#include <immintrin.h>
#define HAS_AVX2 1
#else
#define HAS_AVX2 0
#endif

#ifdef __SSE2__
#include <emmintrin.h>
#define HAS_SSE2 1
#else
#define HAS_SSE2 0
#endif

#define NUM_THREADS 8

static BenchResult MakeResult(const char *name, uint64_t sum, size_t n,
                               uint64_t ns) {
  printf("  Sum: %lu\n", sum);

  BenchResult result = {
      .name = name,
      .iterations = n,
      .total_ns = ns,
      .ns_per_access = (double)ns / n,
  };
  return result;
}

static uint64_t SumNaive(const uint64_t *array, size_t n) {
  uint64_t sum = 0;
  for (size_t i = 0; i < n; i++) {
    sum += array[i];
  }
  return sum;
}

BenchResult BenchReductionNaive(uint64_t *array, size_t n) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = SumNaive(array, n);
  Escape(&sum);

  clock_gettime(CLOCK_MONOTONIC, &end);
  uint64_t ns =
      (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);

  return MakeResult("Reduction Naive (1 accumulator)", sum, n, ns);
}

static uint64_t SumILP(const uint64_t *array, size_t n) {
  uint64_t sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
  uint64_t sum4 = 0, sum5 = 0, sum6 = 0, sum7 = 0;

  size_t i = 0;
  for (; i + 8 <= n; i += 8) {
    sum0 += array[i + 0];
    sum1 += array[i + 1];
    sum2 += array[i + 2];
    sum3 += array[i + 3];
    sum4 += array[i + 4];
    sum5 += array[i + 5];
    sum6 += array[i + 6];
    sum7 += array[i + 7];
  }

  for (; i < n; i++) {
    sum0 += array[i];
  }

  return sum0 + sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7;
}

BenchResult BenchReductionILP(uint64_t *array, size_t n) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = SumILP(array, n);
  Escape(&sum);

  clock_gettime(CLOCK_MONOTONIC, &end);
  uint64_t ns =
      (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);

  return MakeResult("Reduction ILP (8 accumulators)", sum, n, ns);
}

#if HAS_AVX2
static uint64_t SumSimd(const uint64_t *array, size_t n) {
  __m256i vsum = _mm256_setzero_si256();

  size_t i = 0;
  for (; i + 4 <= n; i += 4) {
    __m256i v = _mm256_loadu_si256((const __m256i *)(array + i));
    vsum = _mm256_add_epi64(vsum, v);
  }

  uint64_t tmp[4];
  _mm256_storeu_si256((__m256i *)tmp, vsum);
  uint64_t sum = tmp[0] + tmp[1] + tmp[2] + tmp[3];

  for (; i < n; i++) {
    sum += array[i];
  }
  return sum;
}
#elif HAS_SSE2
static uint64_t SumSimd(const uint64_t *array, size_t n) {
  __m128i vsum = _mm_setzero_si128();

  size_t i = 0;
  for (; i + 2 <= n; i += 2) {
    __m128i v = _mm_loadu_si128((const __m128i *)(array + i));
    vsum = _mm_add_epi64(vsum, v);
  }

  uint64_t tmp[2];
  _mm_storeu_si128((__m128i *)tmp, vsum);
  uint64_t sum = tmp[0] + tmp[1];

  for (; i < n; i++) {
    sum += array[i];
  }
  return sum;
}
#else
static uint64_t SumSimd(const uint64_t *array, size_t n) {
  return SumILP(array, n);
}
#endif

BenchResult BenchReductionSimd(uint64_t *array, size_t n) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = SumSimd(array, n);
  Escape(&sum);

  clock_gettime(CLOCK_MONOTONIC, &end);
  uint64_t ns =
      (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);

  const char *name = HAS_AVX2 ? "Reduction SIMD (AVX2 4x64)"
                              : (HAS_SSE2 ? "Reduction SIMD (SSE2 2x64)"
                                          : "Reduction SIMD (fallback)");
  return MakeResult(name, sum, n, ns);
}

typedef struct {
  const uint64_t *array;
  size_t start;
  size_t end;
  uint64_t result;
} ThreadArg;

static void *ThreadSum(void *arg) {
  ThreadArg *ta = (ThreadArg *)arg;
  uint64_t sum = 0;
  for (size_t i = ta->start; i < ta->end; i++) {
    sum += ta->array[i];
  }
  ta->result = sum;
  return NULL;
}

static uint64_t SumThreaded(const uint64_t *array, size_t n) {
  pthread_t threads[NUM_THREADS];
  ThreadArg args[NUM_THREADS];

  size_t chunk_size = n / NUM_THREADS;
  size_t remainder = n % NUM_THREADS;

  size_t offset = 0;
  for (int t = 0; t < NUM_THREADS; t++) {
    args[t].array = array;
    args[t].start = offset;
    size_t this_chunk = chunk_size + (t < (int)remainder ? 1 : 0);
    args[t].end = offset + this_chunk;
    args[t].result = 0;
    offset = args[t].end;
    pthread_create(&threads[t], NULL, ThreadSum, &args[t]);
  }

  uint64_t sum = 0;
  for (int t = 0; t < NUM_THREADS; t++) {
    pthread_join(threads[t], NULL);
    sum += args[t].result;
  }
  return sum;
}

BenchResult BenchReductionThread(uint64_t *array, size_t n) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = SumThreaded(array, n);
  Escape(&sum);

  clock_gettime(CLOCK_MONOTONIC, &end);
  uint64_t ns =
      (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);

  return MakeResult("Reduction Threaded (8 threads)", sum, n, ns);
}

#if HAS_AVX2
static uint64_t SumILPSimd(const uint64_t *array, size_t n) {
  __m256i vsum0 = _mm256_setzero_si256();
  __m256i vsum1 = _mm256_setzero_si256();
  __m256i vsum2 = _mm256_setzero_si256();
  __m256i vsum3 = _mm256_setzero_si256();

  size_t i = 0;
  for (; i + 16 <= n; i += 16) {
    __m256i v0 = _mm256_loadu_si256((const __m256i *)(array + i + 0));
    __m256i v1 = _mm256_loadu_si256((const __m256i *)(array + i + 4));
    __m256i v2 = _mm256_loadu_si256((const __m256i *)(array + i + 8));
    __m256i v3 = _mm256_loadu_si256((const __m256i *)(array + i + 12));
    vsum0 = _mm256_add_epi64(vsum0, v0);
    vsum1 = _mm256_add_epi64(vsum1, v1);
    vsum2 = _mm256_add_epi64(vsum2, v2);
    vsum3 = _mm256_add_epi64(vsum3, v3);
  }

  vsum0 = _mm256_add_epi64(vsum0, vsum1);
  vsum2 = _mm256_add_epi64(vsum2, vsum3);
  vsum0 = _mm256_add_epi64(vsum0, vsum2);

  uint64_t tmp[4];
  _mm256_storeu_si256((__m256i *)tmp, vsum0);
  uint64_t sum = tmp[0] + tmp[1] + tmp[2] + tmp[3];

  for (; i < n; i++) {
    sum += array[i];
  }
  return sum;
}
#else
static uint64_t SumILPSimd(const uint64_t *array, size_t n) {
  return SumILP(array, n);
}
#endif

BenchResult BenchReductionILPSimd(uint64_t *array, size_t n) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = SumILPSimd(array, n);
  Escape(&sum);

  clock_gettime(CLOCK_MONOTONIC, &end);
  uint64_t ns =
      (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);

  const char *name =
      HAS_AVX2 ? "Reduction ILP+SIMD (4xAVX2)" : "Reduction ILP+SIMD (fallback)";
  return MakeResult(name, sum, n, ns);
}

static void *ThreadSumILPSimd(void *arg) {
  ThreadArg *ta = (ThreadArg *)arg;
#if HAS_AVX2
  __m256i vsum0 = _mm256_setzero_si256();
  __m256i vsum1 = _mm256_setzero_si256();

  size_t i = ta->start;
  for (; i + 8 <= ta->end; i += 8) {
    __m256i v0 = _mm256_loadu_si256((const __m256i *)(ta->array + i + 0));
    __m256i v1 = _mm256_loadu_si256((const __m256i *)(ta->array + i + 4));
    vsum0 = _mm256_add_epi64(vsum0, v0);
    vsum1 = _mm256_add_epi64(vsum1, v1);
  }

  vsum0 = _mm256_add_epi64(vsum0, vsum1);
  uint64_t tmp[4];
  _mm256_storeu_si256((__m256i *)tmp, vsum0);
  uint64_t sum = tmp[0] + tmp[1] + tmp[2] + tmp[3];

  for (; i < ta->end; i++) {
    sum += ta->array[i];
  }
  ta->result = sum;
#else
  uint64_t sum = 0;
  for (size_t i = ta->start; i < ta->end; i++) {
    sum += ta->array[i];
  }
  ta->result = sum;
#endif
  return NULL;
}

static uint64_t SumAll(const uint64_t *array, size_t n) {
  pthread_t threads[NUM_THREADS];
  ThreadArg args[NUM_THREADS];

  size_t chunk_size = n / NUM_THREADS;
  size_t remainder = n % NUM_THREADS;

  size_t offset = 0;
  for (int t = 0; t < NUM_THREADS; t++) {
    args[t].array = array;
    args[t].start = offset;
    size_t this_chunk = chunk_size + (t < (int)remainder ? 1 : 0);
    args[t].end = offset + this_chunk;
    args[t].result = 0;
    offset = args[t].end;
    pthread_create(&threads[t], NULL, ThreadSumILPSimd, &args[t]);
  }

  uint64_t sum = 0;
  for (int t = 0; t < NUM_THREADS; t++) {
    pthread_join(threads[t], NULL);
    sum += args[t].result;
  }
  return sum;
}

BenchResult BenchReductionAll(uint64_t *array, size_t n) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t sum = SumAll(array, n);
  Escape(&sum);

  clock_gettime(CLOCK_MONOTONIC, &end);
  uint64_t ns =
      (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);

  return MakeResult("Reduction All (8 threads + ILP + SIMD)", sum, n, ns);
}
