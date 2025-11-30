#
# Makefile for server for CS50
# 
# JL3 Fall 24

L = ../libcs50
S = ../support
G = ../game

OBJS = server.o $G/game.a 
LIBS = -lm 
LLIBS = $L/libcs50.a $S/support.a $G/game.a

# Flags
CFLAGS = -Wall -pedantic -std=c11 -g -ggdb -I$L -I$S -I$G
CC = gcc 
Make = make

ALL: server

server: $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $(OBJS) $(LLIBS) $(LIBS) -o $@

#Dependencies
server.o: $G/game.h

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f vgcore*
	rm -f server
