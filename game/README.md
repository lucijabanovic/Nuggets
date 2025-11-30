# Nuggets, JL3
## CS50 Fall 2024

### Game 
Game provides the two core data structures server relies on to play the game.
Grid allows us easy mathematical operations of treating a char* like a height x width grid.
Game allows us to update the game state and make any necessary moves.


### Usage
We have two many structures in game:

Usage of the game module:
```c
game_t* game_new(char* mapPath);
bool game_addPlayer(game_t* game, char* name, addr_t address);
bool game_deletePlayer(game_t* game, char* addressStr);
bool game_addSpectator(game_t* game, addr_t address);
addr_t* game_getSpectatorAddress(game_t* game);
bool game_deleteSpectator(game_t* game);
bool game_move(game_t* game, char* addressStr, int cx, int cy);
void game_sendDisplays(game_t* game);
void game_updateVisibility(game_t* game);
int game_getRemainingGold(game_t* game);
void game_delete(game_t* game);
void game_end(game_t* game);
```

Usage of the grid module:
```c
grid_t* grid_new(int height, int width);
int grid_getHeight(grid_t* grid);
int grid_getWidth(grid_t* grid);
char grid_getChar(grid_t* grid, char* map, int x, int y);
bool grid_putChar(grid_t* grid, char* map, int x, int y, char c);
void grid_display(grid_t* grid, char* map);
void grid_delete(grid_t* grid);
void grid_addVisiblePoints(grid_t *grid, char *mapOG, char* mapCurr, char *visibleMap, int px, int py);
```

## Assumptions
Any assumptions we make about functions called are discussed in the h files.

### Files
* `Makefile` - compilation procedure
* `game.c` - the implementation of game
* `gametest.c` - the testing for game
* `game.h` - usage for game module 
* `grid.c` - the implementation of grid 
* `gridtest.c` - the testing for grid
* `grid.h` - usage for grid module 

### Compilation
To compile, simply `make`.

### Clean
Simple type `make clean`
