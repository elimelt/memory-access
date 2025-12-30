# Makefile - Auto-discovers bench_*.c files from subdirectories

CC = gcc
CFLAGS = -O3 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=199309L -march=native -I.
LDFLAGS = -lrt -lpthread
TARGET = bench

BENCH_SRCS := $(wildcard */bench_*.c)
BENCH_OBJS := $(BENCH_SRCS:.c=.o)

MAIN_OBJ = main.o
ALL_OBJS = $(MAIN_OBJ) $(BENCH_OBJS)

all: $(TARGET)

$(TARGET): $(ALL_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ALL_OBJS) $(LDFLAGS)

$(MAIN_OBJ): main.c bench.h
	$(CC) $(CFLAGS) -c $< -o $@

%/bench_%.o: %/bench_%.c bench.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(MAIN_OBJ) $(BENCH_OBJS)

.PHONY: all clean

info:
	@echo "Discovered benchmark sources:"
	@echo "  $(BENCH_SRCS)"
	@echo "Object files:"
	@echo "  $(ALL_OBJS)"
