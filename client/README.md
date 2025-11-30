# Nuggets, JL3
## CS50 Fall 2024

### Client 
The client module interfaces with the server through messages to allow
a user to play the game.

### Usage

Run one or the other, depending on whether you want to be a player or a spectator:
```
./client 2>player.log hostname port playername
./client 2>spectator.log hostname port
```

This will print the client log into a log file.

### Client module functions

```c
int main(const int argc, char *argv[]); 
static bool sendKeystrokes(void *arg);
static bool parseMessage(void *arg, const addr_t from, const char *message);
```

## Assumptions
We assume the user exits their terminal upon successful completion of the game

### Files
* `Makefile` - compilation procedure
* `client.c` - the implementation of game

### Compilation
To compile, simply `make`.

### Clean
Simply type `make clean`
