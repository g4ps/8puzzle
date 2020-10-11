CFLAGS = -g 
CC=gcc
CFLAGS =
a.out: puzzle.o
	$(CC) $^ $(CFLAGS) -o $@

.PHONY: clean
clean:
	rm a.out *.o
