CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LIBS =
EXEC = lh
SRC = lh.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LIBS)

clean:
	rm -f $(EXEC)
