#
# Makefile for grid_n_game for CS50
# 
# JL3 Fall 24

LIB = game.a

L = ../libcs50
S = ../support
OBJS = gridtest.o grid.o
OBJSg = gametest.o game.o grid.o
LIBS = -lm 
LLIBS = $L/libcs50.a $S/support.a 

CFLAGS = -Wall -pedantic -std=c11 -g -ggdb -I$L -I$S
CC = gcc 

all: $(LIB) gametest gridtest

$(LIB): game.o grid.o 
	ar cr $(LIB) $^

gridtest: $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $(OBJS) $(LLIBS) $(LIBS) -o $@

gametest: $(OBJSg) $(LLIBS)
	$(CC) $(CFLAGS) $(OBJSg) $(LLIBS) $(LIBS) -o $@
	

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f gametest
	rm -f vgcore*
	rm -f gridtest
	rm -f $(LIB)
