CC=gcc  #compiler
CFLAGS=-Wall -Wextra -std=c11 -O2 #compilation flags

img-search: main.o 
	$(CC) main.o -g -o img-search 

main_test.o : main.c utils.h
	$(CC) $(CFLAGS) -c main.c -lrt


clean:
	rm *.o img-search

