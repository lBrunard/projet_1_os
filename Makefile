CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -O2

img-search: main.o
	$(CC) main.o -o img-search

main_test.o : main.c
	$(CC) $(CFLAGS) -c main.c


clean:
	rm *.o img-search

# $@ = target file
# $^ = all dependencies
# $< = first dependency
# $* = name of target without extension
# $% = filename element of an archive member specification
# $? = all dependencies newer than target

#target : dependecies
#   actions