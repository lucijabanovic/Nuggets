# CS50 Nuggets
## Design Spec
### JL3, Fall, 2024

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes grid, message and game state modules for the server. 
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

## Current plan for splitting up work

1. Lucija - Client
2. Jack - Grid and visibility + Game support
3. Lizzy - Message handling + Game support
4. Lukas - Game lead

## Player
The *client* acts in one of two modes:

 1. *spectator*, the passive spectator mode described in the requirements spec.
 2. *player*, the interactive game-playing mode described in the requirements spec.

### User interface
See the requirements spec for both the command-line and interactive UI.

### Inputs and outputs
Inputs (Keystrokes):

1. Q: quit the game

Movement (if player)

1. h: move left, if possible
2. l: move right, if possible
3. j: move down, if possible
4. k: move up, if possible
5. y: move diagonally up and left, if possible
6. u: move diagonally up and right, if possible
7. b: move diagonally down and left, if possible
8. n: move diagonally down and right, if possible

Output (Display):

1. Game Grid: Map of players, gold, and rooms visible to player
2. Status Line: One line at top of display with game specific updates or errors
3. Game Over Summary: List of players and amount of gold collected

### Functional decomposition into modules

Our nuggets client is comprised of 3 main modules:

1. parseMessage, to handle incoming messages from server
2. sendKeystroke, to handle sending user input to server
3. display, displays the game in its current state, as visible to player/spectator

### Pseudo code for logic/algorithmic flow
The client will run as follow:
```
validate command line arguments
determine if player or spectator
initialize display and network
while handling messages from server (until a quit message)
    display the grid and any status lines
    send any user keystrokes 
display game over summary
```

#### parseMessage
Pseudocode for parseMessage:
```
receive message from server
parse for type of message (GRID, GOLD, DISPLAY, QUIT, OK)
execute according to message type
```

#### grid_display
Pseudocode described later in grid module

#### sendKeystrokes
Pseudocode for sendKeystrokes:
```
receive keystroke from user
create message to send to server
send to server with provided message module
```

### Major data structures
We use 1 main data structure on the client side.

1. Grid, to display the grid

We could potentially create a local player data structure to store player information,
though this information could also simply be stored in separate variables.

---

## Server
### User interface

See the requirements spec for the command-line interface.
There is no interaction with the user.

### Inputs and outputs
Inputs (map file)

1.  map.txt file, if map document isn't readable, exit with error 

Inputs (recieved from client)

1. PLAY, SPECTATE: result from intitializing a client
2. KEY: results from player keyboard input

Output (to terminal)

1. log module - info about user joining game/leaving game and collecting gold

### Functional decomposition into modules
Our nuggets server is comprised of 3 main modules:

1. Grid, a data structure containing the grid size parameters, map string, and array of points in the game map 
2. Message, in charge of sending and receiving messages between client and server
3. Game State, in charge of game play operations (new player, move, ...)

### Pseudo code for logic/algorithmic flow

The server will run as follows:
```
execute from a command line per the requirement spec
parse the command line, validate parameters
call initializeGame() to set up data structures
initialize the 'message' module
print the port number on which we wait
call message_loop(), to await clients
call gameOver() to inform all clients the game has ended
clean up
```

#### initializeGame
Pseudocode for initializeGame
```
Create Game structure
Call initializeMap on Game structure, which intializes grid
```

#### message_loop
Pseudocode for message_loop
```
while game is going (not all gold collected or no q KEY)
   validate message
   get key from client
   switch to that player
   call function for player based on keystroke
   update all clients on game state
```

#### gameOver
Pseudocode for gameOver
```
send result message to all clients
clean up data structures
```

### Major data structures
We use 5 main data structures:

1. Map, pointer to two grids: one for the original (base) version and one for the current game play version.
2. Grid, contains a list of points, a string representing the map, and the size
3. Player, contains an ID, Name, purse, and current x,y coordinates
4. Pile, for tracking gold piles. contains an x,y coordinate and an amount
5. Game, structure that contains all of the above data (a map, array of players, array of piles)

---

## Grid module
### Functional decomposition

* grid_initialize, put string into variable and convert string to array of points
* grid_getSize, get a coordinate of grid width, height
* grid_getChar, get char from point
* grid_putChar, put char at point
* grid_display, display map
* grid_delete, clean up the grid memory

### Pseudo code for logic/algorithmic flow
#### grid_initialize
The pseudocode for grid_initialize
```
set width and height
save map in string
create list of points
```

#### grid_getChar
The pseudocode for grid_getChar
```
take in x,y point
find that point in array
return char
```

#### grid_putChar
The pseudocode for grid_putChar
```
take in x,y point
find that point in array
put the desired char there
return true if happens, false if not possible
```

#### grid_display
The pseudocode for grid_display
```
loop through string
   place new line at every width letters
```

#### grid_delete
```
free list
free string
free grid
```

### Major data structures
We use two main data structures:

1. Grid structure
2. List of points to keep track of where the player can see

---
## Game module
### Functional decomposition
* initializeMap, creates map and grids contained adds gold
* move, moves player based on key stroke 
* addPlayer, adds player to game
* deletePlayer, removes player from game
* addSpectator, adds spectator to game
* deleteSpectator, removes spectator from game

### Pseudo code for logic/algorithmic flow

#### initializeMap
Pseudocode for initializeMap
```
create map structure
create the two grids that map points to
randomly generate gold amounts and points
change grids char* to reflect gold
put gold amounts in gold array
```
    
#### move
Pseudocode for move
```
check that move is valid (on point in map)
check if gold at move location
    add gold to player score
    swap spots of move to and move from points using putChar
    update visibility of player
```

### Major data structures

* Grid, structure as referenced abov
* Map, pointer to two grids (base and current)
* Player, location, score, name, ID
* Gold, location and amount
* List of gold structs, to keep track of gold pile locations  

## Message module
### Functional decomposition

* message_sendOK, if initialized as player
* messsage_sendGRID, sends dimensions of grid needed
* message_sendGOLD, amount of gold messages
* message_sendDISPLAY, send string state of map
* message_sendQUIT, sends quit message
* message_sendERROR, sends error message

### Pseudo code for logic/algorithmic flow
Each of these functions has similar, simple pseudo code of forming the message and then invoking the client-server
message file.

## Testing Plan
We iteratively test through the construction of the program, specifically with the grid module and making sure all works
as desired. Then we:

1. test the programs with various incorrect command line inputs
2. test the programs with the programs provided to us
3. test the programs by playing together and attempting to break, reviewing game logs
