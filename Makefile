CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -O2

output: main.o img-search.o
    gcc main.o img-search.o -o output

main.o : main.cpp
    gcc $(CFLAGS) -c main.cpp


img-search.o : img-search.cpp
    gcc $(CFLAGS) -c message.cpp


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