CC=gcc
CFLAGS=-Wall -lreadline -ljson-c
INSTALL=install
PREFIX=/usr

all: lh

lh: lh.c
	$(CC) lh.c $(CFLAGS) -o lh

install:
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 755 lh $(DESTDIR)$(PREFIX)/bin/
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/doc/lh
	$(INSTALL) -m 644 LICENSE $(DESTDIR)$(PREFIX)/share/doc/lh/
	$(INSTALL) -m 644 README.md $(DESTDIR)$(PREFIX)/share/doc/lh/

clean:
	rm -f lh

.PHONY: all install clean
