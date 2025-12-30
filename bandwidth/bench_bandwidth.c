#include "bench.h"

#include <pthread.h>
#include <stdio.h>
#include <time.h>

typedef struct {
  uint64_t *start;
  size_t count;
  uint64_t result;
} ThreadArg;

static void *SumThread(void *arg) {
  ThreadArg *ta = (ThreadArg *)arg;
  uint64_t sum = 0;
  uint64_t *arr = ta->start;
  size_t n = ta->count;

  for (size_t i = 0; i < n; i++) {
    sum += arr[i];
  }

  ta->result = sum;
  return NULL;
}

static BenchResult RunBandwidth(uint64_t *array, size_t n, int num_threads, const char *name) {
  pthread_t threads[32];
  ThreadArg args[32];
  size_t chunk = n / num_threads;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  for (int t = 0; t < num_threads; t++) {
    args[t].start = array + t * chunk;
    args[t].count = chunk;
    pthread_create(&threads[t], NULL, SumThread, &args[t]);
  }

  uint64_t total = 0;
  for (int t = 0; t < num_threads; t++) {
    pthread_join(threads[t], NULL);
    total += args[t].result;
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  Escape(&total);

  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
  double seconds = ns / 1e9;
  double gb = (n * sizeof(uint64_t)) / 1e9;
  double gbps = gb / seconds;

  printf("  Bandwidth: %.1f GB/s\n", gbps);

  return (BenchResult){name, n, ns, (double)ns / n};
}

BenchResult BenchBw1(uint64_t *a, size_t n) { return RunBandwidth(a, n, 1, "Bandwidth 1 thread"); }
BenchResult BenchBw2(uint64_t *a, size_t n) { return RunBandwidth(a, n, 2, "Bandwidth 2 threads"); }
BenchResult BenchBw4(uint64_t *a, size_t n) { return RunBandwidth(a, n, 4, "Bandwidth 4 threads"); }
BenchResult BenchBw8(uint64_t *a, size_t n) { return RunBandwidth(a, n, 8, "Bandwidth 8 threads"); }

