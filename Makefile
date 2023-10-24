CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -O2

output: main.o img-search.o
	$(CC) main.o img-search.o -o output

main.o : main.c
	$(CC) $(CFLAGS) -c main.c


img-search.o : img-search.c
	$(CC) $(CFLAGS) -c img-search.c


clean:
	rm *.o output

# $@ = target file
# $^ = all dependencies
# $< = first dependency
# $* = name of target without extension
# $% = filename element of an archive member specification
# $? = all dependencies newer than target

#target : dependecies
#   actions