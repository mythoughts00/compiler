CC=gcc
CFLAGS=-Wall -g -o
MAIN=main.c fatal.c hash.c debug.c
FRONT=token.c identifier.c parser.c #priority.c

install:
	$(CC) $(FRONT) $(MAIN) $(CFLAGS) scc

clean:
