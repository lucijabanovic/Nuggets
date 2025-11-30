#
# Makefile for nuggets game for CS50
# 
# JL3 Fall 24

L = ./libcs50
S = ./support
SE = ./server
G = ./game
OBJSs = $(SE)/server.c $G/grid.o $G/game.o
LIBS = -lm 
LLIBS = $L/libcs50.a $S/support.a 

CFLAGS = -Wall -pedantic -std=c11 -g -ggdb -I$L -I$S -I$G
CC = gcc 

all: 
	(cd $L && make $L.a)
	(cd $S && make $S.a)
	make -C game
	make -C server
	make -C client

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f vgcore*
	(cd ./client && make clean)
	(cd $G && make clean)
	(cd $(S) && make clean)
	(cd $(L) && make clean)
	(cd $(SE) && make clean)
