CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LIBS = -lreadline -ljson-c
EXEC = lh
SRC = src/lh.c

all:
    $(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LIBS)  # Make sure this line starts with a tab, not spaces

clean:
    rm -f $(EXEC)  # Make sure this line starts with a tab, not spaces
