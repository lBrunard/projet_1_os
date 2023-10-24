CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -O2
LDFLAGS=-lm
OBJS=img-search.o vector.o

all: img-search

img-search: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

img-search.o: img-search.c vectors/vector.h
	$(CC) $(CFLAGS) -c $<

vector.o: vectors/vector.c vectors/vector.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) img-search