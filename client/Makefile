
CFLAGS = -Wall -pedantic -std=c11 -ggdb -I../support
CC = gcc

LIBS = -lncurses
OBJ = client.o ../support/support.a
EXEC = client

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXEC) *.o .*
