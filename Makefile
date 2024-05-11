CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LIBS = -lreadline -ljson-c
EXEC = lh
SRC = src/lh.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LIBS)

clean:
	rm -f $(EXEC)
