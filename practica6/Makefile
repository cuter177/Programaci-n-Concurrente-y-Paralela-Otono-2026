CC     = gcc
CFLAGS = -O2 -Wall -Wextra -fopenmp
LDLIBS = -lm

programa: programa.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

run: programa
	./programa

clean:
	rm -f programa

.PHONY: run clean
