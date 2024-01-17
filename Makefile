CC = gcc
AR = ar
CFLAGS = -Wall

all: main

mymalloc.o: mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -c mymalloc.c -o mymalloc.o

libmymalloc.a: mymalloc.o
	$(AR) rc libmymalloc.a mymalloc.o

main: main.c libmymalloc.a
	$(CC) $(CFLAGS) -static main.c -o main -L . -lmymalloc

clean:
	rm -f mymalloc.o libmymalloc.a main
