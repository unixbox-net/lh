CC=gcc
CFLAGS=-Wall

all: lh

lh: src/lh.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f lh

install:
	install -m 0755 lh /usr/local/bin/lh

.PHONY: all clean install
