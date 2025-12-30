# Parallel Reductions Benchmark: Demonstrating ILP

This benchmark suite demonstrates **Instruction-Level Parallelism (ILP)** and other optimization techniques through array reduction (summing all elements).

## The Problem with Naive Reduction

A naive reduction creates a **dependency chain**:

```c
uint64_t sum = 0;
for (size_t i = 0; i < n; i++) {
    sum += array[i];  // Each add depends on the previous!
}
```

Each addition must wait for the previous one to complete. On a modern CPU that can execute 4+ adds per cycle, this leaves most execution units idle.

## Benchmark Variants

| Variant | Description |
|---------|-------------|
| `red_naive` | Single accumulator - creates dependency chain |
| `red_ilp` | 8 independent accumulators - breaks dependency chain |
| `red_simd` | AVX2 vector adds (4 × 64-bit per instruction) |
| `red_thread` | 8 pthreads, each summing a portion |
| `red_ilp_simd` | 4 independent AVX2 accumulators |
| `red_all` | Threads + ILP + SIMD combined |

## Results

### Performance by Array Size (ns per element)

| Size | naive | ilp | simd | thread | ilp+simd | all |
|------|-------|-----|------|--------|----------|-----|
| **1 MB** | 0.41 | 0.21 | 0.17 | 1.70 | 0.15 | 1.40 |
| **4 MB** | 0.21 | 0.19 | 0.18 | 0.37 | 0.21 | 0.35 |
| **16 MB** | 0.35 | 0.27 | 0.24 | 0.18 | 0.25 | 0.15 |
| **64 MB** | 0.41 | 0.31 | 0.30 | 0.19 | 0.30 | 0.17 |
| **256 MB** | 0.40 | 0.31 | 0.33 | 0.17 | 0.32 | 0.14 |
| **1024 MB** | 0.39 | 0.31 | 0.32 | 0.15 | 0.32 | 0.13 |

### Speedup vs Naive (1024 MB)

| Variant | Time (ms) | ns/elem | Speedup |
|---------|-----------|---------|---------|
| `red_naive` | 52.8 | 0.39 | 1.0× |
| `red_ilp` | 41.8 | 0.31 | **1.3×** |
| `red_simd` | 43.3 | 0.32 | **1.2×** |
| `red_thread` | 19.5 | 0.15 | **2.7×** |
| `red_ilp_simd` | 42.4 | 0.32 | **1.2×** |
| `red_all` | 16.9 | 0.13 | **3.1×** |

## Key Observations

### 1. ILP Benefit (~1.3× speedup)

The 8-accumulator version breaks the dependency chain:

```c
// 8 independent chains - CPU can execute in parallel
sum0 += array[i+0];
sum1 += array[i+1];
sum2 += array[i+2];
// ... all 8 can execute simultaneously
```

The ~1.3× speedup (not 8×) indicates the workload is **memory-bandwidth bound**, not compute-bound.

### 2. Small Arrays: Threading Hurts

At 1-4 MB (fits in cache), threading overhead dominates:
- `red_naive`: 0.21-0.41 ns
- `red_thread`: 0.37-1.70 ns (slower!)

Thread creation/synchronization costs ~100-200 μs, which dominates for small workloads.

### 3. Large Arrays: Threading Wins

At 64+ MB (exceeds cache), threading provides the biggest benefit:
- `red_naive`: 0.39-0.41 ns
- `red_thread`: 0.15-0.19 ns (**2.5-2.7× faster**)

Multiple threads can saturate memory bandwidth from different memory controllers/channels.

### 4. SIMD ≈ ILP for this Workload

SIMD and scalar ILP show similar performance because:
- Both break the dependency chain
- Memory bandwidth is the bottleneck, not ALU throughput
- AVX2 loads 32 bytes/instruction, but memory can't keep up

### 5. Cache Effects

Performance varies with array size due to cache hierarchy:

| Size | Likely Location | Observed Behavior |
|------|-----------------|-------------------|
| 1 MB | L2/L3 cache | Fastest single-thread; threading hurts |
| 4 MB | L3 cache | ILP/SIMD help; threading overhead visible |
| 16+ MB | Main memory | Threading helps; memory-bound |

## Conclusion

For **compute-bound** workloads, ILP techniques (multiple accumulators, SIMD) can provide significant speedups by keeping the CPU's execution units busy.

For **memory-bound** workloads like large array reduction, the bottleneck is memory bandwidth, not CPU compute. Multi-threading helps by utilizing multiple memory channels, while ILP/SIMD provide modest benefits.

The best strategy depends on:
1. **Working set size** relative to cache
2. **Memory bandwidth** vs compute intensity
3. **Thread overhead** vs workload size

## Running the Benchmarks

```bash
# Build
make

# Run individual benchmark
./bench red_naive 256    # 256 MB array
./bench red_ilp 256
./bench red_all 256

# Compare all variants
for v in red_naive red_ilp red_simd red_thread red_ilp_simd red_all; do
    ./bench $v 256 2>&1 | grep "Time per"
done
```

