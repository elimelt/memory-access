#!/bin/bash
# run_benchmarks.sh

set -e

# Check if bench binary exists
if [ ! -f ./bench ]; then
  echo "Error: bench binary not found. Run 'make' first."
  exit 1
fi

# Default size in MB
SIZE=${1:-4096}

echo "=================================="
echo "Memory Access Pattern Benchmarks"
echo "Array Size: ${SIZE} MB"
echo "=================================="
echo ""

# Run each benchmark with perf stats
for benchmark in seq ran chase; do
  echo "Running: $benchmark"
  echo "---"
  perf stat -e LLC-loads,LLC-load-misses ./bench $benchmark $SIZE 2>&1 | tee ${benchmark}_${SIZE}mb.log
  echo ""
  echo "---"
  echo ""
done

echo "=================================="
echo "Results Summary"
echo "=================================="
echo ""

# Extract key metrics from logs
for benchmark in seq ran chase; do
  log="${benchmark}_${SIZE}mb.log"
  if [ -f "$log" ]; then
    echo "=== $benchmark ==="
    grep "Time per access:" $log
    grep "LLC-load-misses" $log | grep -v "cpu_atom"
    echo ""
  fi
done

echo "Full logs saved as: seq_${SIZE}mb.log, ran_${SIZE}mb.log, chase_${SIZE}mb.log"
