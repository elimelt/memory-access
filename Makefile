# Makefile
CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=199309L -march=native
LDFLAGS = -lrt -lpthread
TARGET = bench

all: $(TARGET)

$(TARGET): main.o bench_sequential.o bench_random.o bench_pointer_chase.o bench_reduction.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o bench_sequential.o bench_random.o bench_pointer_chase.o bench_reduction.o $(LDFLAGS)

main.o: main.c bench.h
	$(CC) $(CFLAGS) -c main.c

bench_sequential.o: bench_sequential.c bench.h
	$(CC) $(CFLAGS) -c bench_sequential.c

bench_random.o: bench_random.c bench.h
	$(CC) $(CFLAGS) -c bench_random.c

bench_pointer_chase.o: bench_pointer_chase.c bench.h
	$(CC) $(CFLAGS) -c bench_pointer_chase.c

bench_reduction.o: bench_reduction.c bench.h
	$(CC) $(CFLAGS) -c bench_reduction.c

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
