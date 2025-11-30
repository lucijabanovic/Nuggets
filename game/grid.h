/* 
 * grid.h - header file for nuggets project grid module
 *
 * A *grid* holds the dimensions of a map and can modify said map 
 *
 * Jack Beecher, CS50, Fall 2024 
 * 
 */

#ifndef __grid_H
#define __grid_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "mem.h"
#include "file.h"

/**************** global types ****************/
typedef struct grid grid_t;

/**************** functions ****************/

/**************** grid_new ****************/
/* Create a grid structure 
 *
 * Caller provides:
 *   height and width of grid 
 * We return:
 *   pointer to the new grid; return NULL if error.
 * Caller is responsible for:
 *   later calling grid_delete.
 *
 */
grid_t* grid_new(int height, int width);

/**************** grid_getHeight ****************/
/*
 * Getter method for grid height
 *
 * Caller provides:
 *   game
 * We return:
 *   int of the height
 */
int grid_getHeight(grid_t* grid);

/**************** grid_getWidth ****************/
/*
 * Getter method for grid width 
 *
 * Caller provides:
 *   game
 * We return:
 *   int of the width 
 */
int grid_getWidth(grid_t* grid);

/**************** grid_getChar ****************/
/* get a character from a location of a map 
 *
 * Caller provides:
 *   grid strucure
 *   map string
 *   location (x, y)
 * We return:
 *   char at that point 
 *
 */
char grid_getChar(grid_t* grid, char* map, int x, int y);

/**************** grid_putChar ****************/
/* put a character at a certain location in a map 
 *
 * Caller provides:
 *   grid strucure
 *   map string
 *   location (x, y)
 * We return:
 *   true, if char is placed at that point 
 *   false, if char is unable to be placed at that spot
 * 
 */
bool grid_putChar(grid_t* grid, char* map, int x, int y, char c);

/**************** grid_display ****************/
/* display the map onto the size of the grid 
 *
 * Caller provides:
 *   grid strucure
 *   map string
 * We do:
 *   put map string onto grid dimensions
 * We return:
 *   nothing
 *
 */
void grid_display(grid_t* grid, char* map);

/**************** grid_delete ****************/
/* delete the grid by freeing the pointers 
 *
 * Caller provides:
 *   grid strucure
 * We do:
 *   a free on the grid 
 * We return:
 *   nothing
 *
 */
void grid_delete(grid_t* grid);

/**************** grid_addVisiblePoints ****************/
/* adds visible points to visibleMap string (for a given player)
 *
 * Caller provides:
 *   grid, current map, visible map (where the visible map string goes), and current x and y for player
 * We do:
 *   set visibleMap to the correct string
 * We return:
 *   nothing
 */
void grid_addVisiblePoints(grid_t *grid, char *mapOG, char* mapCurr, char *visibleMap, int px, int py);

#endif // __GRID_H
