# Nuggets, JL3
## CS50 Fall 2024

### Server
Server interfaces with game and client to host a game 

### Usage
Functions in server:
```c
int main(int argc, char* argv[]);
static void parseArgs(const int argc, char* argv[], game_t** game);
static bool handleMessage(void* arg, const addr_t from, const char* message); 
static bool handleKeypress(addr_t from, char key, game_t* game);
```

## Assumptions
None

### Files
* `Makefile` - compilation procedure
* `server.c` - the implementation of game

### Compilation
To compile, simply `make`.

### Clean
Simple type `make clean`
