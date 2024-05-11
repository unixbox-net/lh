CC=gcc
CFLAGS=-Wall -lreadline -ljson-c

all: lh

lh: lh.c
    $(CC) -o lh lh.c $(CFLAGS)

clean:
    rm -f lh
