CC     = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -fopenmp
LDLIBS = -lm

all: programa

programa: programa.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

run: programa
	./programa

clean:
	rm -f programa

.PHONY: all run clean
