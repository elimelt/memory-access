#include "bench.h"

#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define NUM_THREADS 8
#define CACHE_LINE 64

typedef struct { uint64_t count; } Packed;
typedef struct { uint64_t count; char pad[CACHE_LINE - sizeof(uint64_t)]; } Padded;

static Packed g_packed[NUM_THREADS];
static Padded g_padded[NUM_THREADS];

typedef struct {
  size_t thread_id;
  size_t iterations;
  int use_padded;
} ThreadArg;

static void *CounterThread(void *arg) {
  ThreadArg *ta = (ThreadArg *)arg;
  size_t iters = ta->iterations;
  size_t tid = ta->thread_id;

  if (ta->use_padded) {
    volatile uint64_t *p = &g_padded[tid].count;
    for (size_t i = 0; i < iters; i++) {
      (*p)++;
    }
  } else {
    volatile uint64_t *p = &g_packed[tid].count;
    for (size_t i = 0; i < iters; i++) {
      (*p)++;
    }
  }
  return NULL;
}

static BenchResult RunContention(uint64_t *array, size_t n, int use_padded, const char *name) {
  (void)array;

  for (int i = 0; i < NUM_THREADS; i++) {
    g_packed[i].count = 0;
    g_padded[i].count = 0;
  }

  size_t iters_per_thread = n * 10;
  pthread_t threads[NUM_THREADS];
  ThreadArg args[NUM_THREADS];

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  for (int t = 0; t < NUM_THREADS; t++) {
    args[t].thread_id = t;
    args[t].iterations = iters_per_thread;
    args[t].use_padded = use_padded;
    pthread_create(&threads[t], NULL, CounterThread, &args[t]);
  }

  uint64_t total = 0;
  for (int t = 0; t < NUM_THREADS; t++) {
    pthread_join(threads[t], NULL);
    total += use_padded ? g_padded[t].count : g_packed[t].count;
  }

  clock_gettime(CLOCK_MONOTONIC, &end);

  Escape(&total);
  printf("  Total count: %lu\n", total);

  uint64_t ns = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
  size_t total_ops = iters_per_thread * NUM_THREADS;

  return (BenchResult){name, total_ops, ns, (double)ns / total_ops};
}

BenchResult BenchFalseSharing(uint64_t *a, size_t n) {
  return RunContention(a, n, 0, "False sharing (packed counters)");
}

BenchResult BenchNoFalseSharing(uint64_t *a, size_t n) {
  return RunContention(a, n, 1, "No false sharing (padded counters)");
}

