CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -O2 -Wpedantic
LDFLAGS=-lrt

OBJ_UTILS = utils.o

img-search: main.c $(OBJ_UTILS)
	$(CC) main.c -o img-search $(OBJ_UTILS) $(LDFLAGS)

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f *.o img-search