/* 
 * grid.c - nuggets project grid module
 *
 * see grid.h for more information.
 *
 * JL3, CS 50, Fall 2024 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "mem.h"
#include "file.h"
#include "grid.h"

/**************** file-local global variables ****************/
static const char roomSpot = '.';
static const char goldSpot = '*';
static const char wallSpotVert = '|';
static const char wallSpotHoriz = '-';
static const char wallSpotCorn = '+';
static const char tunnelSpot = '#';

/**************** global types ****************/
typedef struct grid {
  int height;   // grid number of rows
  int width;    // grid width of a row
} grid_t;

/**************** global functions ****************/
/* that is, visible outside this file */
/* see index.h for comments about exported functions */

/**************** local functions ****************/
/* not visible outside this file */
static bool is_integer(float x);
static bool tunnel_visibility_helper(grid_t* grid, char* mapCurr, char* visibleMap, int px, int py);
static bool is_seeThrough(char c1, char c2);
bool isVisiblePoint(grid_t* grid, char* mapOG , int x, int y, int px, int py);

/**************** grid_new() ****************/
/* see grid.h for description */
grid_t*
grid_new(int height, int width)
{
  if (height <= 0 || width <= 0){
    return NULL;
  }

  grid_t* grid = malloc(sizeof(grid));
  if (grid == NULL){
    return NULL;
  }

  // initialize contents of grid
  grid->height = height;
  grid->width = width;

  return grid;
}

/**************** grid_getHeight() ****************/
/* see grid.h for description */
int
grid_getHeight(grid_t* grid)
{
  return grid->height;
}

/**************** grid_getWidth() ****************/
/* see grid.h for description */
int
grid_getWidth(grid_t* grid)
{
  return grid->width;
}

/**************** grid_getChar() ****************/
/* see grid.h for description */
char
grid_getChar(grid_t* grid, char* map, int x, int y)
{
  if (x < 0 || x >= grid->width){
    return false;
  }
  if (y < 0 || y >= grid->height){
    return false;
  }
  int index = (y * grid->width) + x;
  return map[index];
}

/**************** grid_putChar() ****************/
/* see grid.h for description */
bool
grid_putChar(grid_t* grid, char* map, int x, int y, char c)
{
  if (x < 0 || x >= grid->width){
    return false;
  }
  if (y < 0 || y >= grid->height){
    return false;
  }
  int index = (y * grid->width) + x;
  map[index] = c;
  return true;
}

/**************** grid_display() ****************/
/* see grid.h for description */
void
grid_display(grid_t* grid, char* map)
{
  for (int y = 0; y < grid->height; y++){
    for (int x = 0; x < grid->width; x++){
      int index = (y * grid->width) + x;
      printf("%c", map[index]);
    }
    printf("\n");
  }
} 

/**************** grid_delete() ****************/
/* see grid.h for description */
void
grid_delete(grid_t* grid)
{
  free(grid);
}

/**************** grid_addVisiblePoints() ****************/
/* see grid.h for description */
void 
grid_addVisiblePoints(grid_t* grid, char* mapOG, char* mapCurr, char* visibleMap, int px, int py)
{
  // if player's position is in room or entrance to a room
  if (grid_getChar(grid, mapOG, px, py) != tunnelSpot || tunnel_visibility_helper(grid, mapOG, visibleMap, px, py)){
    // loop through all points
    for (int x = 0; x < grid->width; x++){
      for (int y = 0; y < grid->height; y++){
        // get chars from each map
        char c1 = grid_getChar(grid, mapCurr, x, y);
        char c2 = grid_getChar(grid, visibleMap, x, y);
        char c3 = grid_getChar(grid, mapOG, x, y );
        // if visible from players point
        if (isVisiblePoint(grid, mapOG, x, y, px, py)){
          // update their map
          grid_putChar(grid, visibleMap, x, y, c1);
        }
        // if not visible, and player's map shows another player or gold
        if (!isVisiblePoint(grid, mapOG, x, y, px, py) && (c2 == goldSpot || isalpha(c2))){
          // replace the char with og map
          grid_putChar(grid, visibleMap, x, y, c3);
        }
      }
    }
  // if player is in tunnel
  } else {
    // loop through all points
    for (int x = 0; x < grid->width; x++){
      for (int y = 0; y < grid->height; y++){
        // if player's map doesn't match with og map, update player's map
        char c1 = grid_getChar(grid, visibleMap, x, y);
        char c2 = grid_getChar(grid, mapOG, x, y);
        if (c1 != c2 && c1 != ' '){
          grid_putChar(grid, visibleMap, x, y, c2);
        }
      }
    }
  } 
  // put the @ for the player at their current location
  grid_putChar(grid, visibleMap, px, py, '@');
}


/**************** isVisiblePoint ****************/
/* 
 * checks if a point (x,y) is visible on the map from
 *  player's location (px, py)
 */
bool
isVisiblePoint(grid_t* grid, char* mapOG , int x, int y, int px, int py)
{
  // if same location, return true
  if (x == px && y == py){
    return true;
  }

  // if a empty space, return false
  if (grid_getChar(grid, mapOG , x, y) == ' '){
    return false;
  }

  // on same vertical line
  if (x == px){
    // check each point on the line
    while (y != py){
      if (y < py){
        y++;
      }
      if (y > py){
        y--;
      }

      // if spot isn't room spot, return false
      char c = grid_getChar(grid, mapOG, x, y);   
      if (c != roomSpot){
        return false;
      }
    }
    return true;
  }

  // calculate slope
  float slope = (float)(py - y)/(px - x);

  // on same horizontal line
  if (slope == 0){
    // check each point on the line
    while (abs(px - x) > 1){
      if (x < px){
        x++;
      } else {
        x--;
      }
      // if spot isn't room spot, return false
      char c = grid_getChar(grid, mapOG , x, (int) y);   
      if (c != roomSpot){
        return false;
      }
    }
  }

  // cast to float values of fy and fx to deal with points between grid markers
  float fy = (float)y;
  float fx = (float)x;

  // if line is relatively shallow, move x discretely and y continously
  if (abs(slope) < 1){
    while (abs(px - x) > 1){
      // move x and y
      if (x < px){
        x++;
        fy = (float)(fy + slope);
      }
      if (x > px){
        x--;
        fy = (float)(fy - slope);
      }

      // check if point falls perfectly on grid marker
      if (is_integer(fy)){
        // check that char is room spot
        char c = grid_getChar(grid, mapOG , x, (int)fy);   
        if (c != roomSpot){
          return false;
        }
      // if point falls between grid lines
      } else {
        // check if the two points are see through
        char c1 = grid_getChar(grid, mapOG , x, floor(fy));
        char c2 = grid_getChar(grid, mapOG , x, ceil(fy));
        if (!is_seeThrough(c1, c2)){
          return false;
        }
      } 
    }
  // if line is relatively steep, move y discretely and x continously
  } else {
    while (abs(py - y) > 1){
      if (y < py){
        y++;
        fx = (float)(fx + (1/slope));
      }
      if (y > py){
        y--;
        fx = (float)(fx + (-1/slope));
      }
      // check if point falls perfectly on grid marker
      if (is_integer(fx)){
        // check that char is room spot
        char c = grid_getChar(grid, mapOG , (int)fx, y);   
        if (c != roomSpot && !isalpha(c) && c != goldSpot){
          return false;
        }
      // if point falls between grid lines
      } else {
        // check if the two points are see through
        char c1 = grid_getChar(grid, mapOG , floor(fx), y);
        char c2 = grid_getChar(grid, mapOG , ceil(fx), y);
        if (!is_seeThrough(c1, c2)){
          return false;
        }
      } 
    }
  }
  // point is visible
  return true;
}

/**************** tunnel_visibility_helper ****************/
/* 
 * Checks if a tunnel spot is an entrance to a room
 * Also updates visible map with all tunne spots within 1
 */
static bool 
tunnel_visibility_helper(grid_t* grid, char* mapOG, char* visibleMap, int px, int py){
  bool isEntrance = false;

  // for all points within 1 of current point
  for (int x = px - 1; x <= px + 1; x++){
    for (int y = py - 1; y <= py + 1; y++){

      // if not equal to player's point
      if (x != px || y != py){

        // get the char from the map and mark it as visible
        char c = grid_getChar(grid, mapOG, x, y);
        grid_putChar(grid, visibleMap, x, y, c);

        // if the char is a roomSpot
        if (c == roomSpot){
          isEntrance = true;
        }
      }
    }
  }
  return isEntrance;
}

/**************** is_seeThrough ****************/
/* 
 * Checks if two chars can be seen through by user
 */
static bool
is_seeThrough(char c1, char c2)
{
  if (c1 == ' ' || c2 == ' '){
    return false;
  }

  // if both spots are walls
  if ((c1 == wallSpotHoriz || c1 == wallSpotCorn || c1 == wallSpotVert || c1 == tunnelSpot) && (c2 == wallSpotHoriz || c2 == wallSpotCorn || c2 == wallSpotVert || c2 == tunnelSpot))
  {
    return false;
  }

  return true;

}

/**************** is_integer ****************/
/* 
 * checks if a float is an integer
 */
static bool
is_integer(float x)
{
  float diff = (float)((int)x) - x;
  return fabs(diff) < 0.00001;
}
