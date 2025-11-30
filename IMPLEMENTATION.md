# CS50 Nuggets
## Implementation Spec
### JL3, 24F, 2024

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes a grid and game module.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements and design specs.

## Plan for division of labor

Lucija wrote the client program. 
Jack handled grid and visibility, as well as part of game.
Lukas handled other methods in game and grid, unit testing, and makefiles
Lizzy wrote the outline of server and handled the messaging within server and game
Jack, Lukas and Lizzy wrote the design spec.
All of us wrote the implementation spec.
Jack, Lukas and Lizzy handled the initial testing of the server
Update your plan for distributing the project work among your 3(4) team members.


### Data structures

#### Grid
Grid stores the height, width parameters of the input map, and has methods that pair an input string map with the grid structure do the math to get and put characters from the string map. 
```c
typedef struct grid {
  int height;
  int width;
} grid_t;
```
#### Player
Player stores all the constant and changing parameters of a given client's player. The gold, x, y, visibleMap and isActive attributes are updated
by various actions that result in calls to the game state, i.e. move or quit. 
```c
typedef struct {
  int gold;
  int x;
  int y;
  char icon;
  char* name;
  addr_t address;
  char* visibleMap;
  bool isSpectator;
  bool isActive;
} player_t;
```

## Client

### Data structures

The client module doesn't use any additional data structures.

### Definition of function prototypes

```c
int main(const int argc, char *argv[]);
static bool sendKeystrokes(void *arg);
static bool parseMessage(void *arg, const addr_t from, const char *message);
```

### Detailed pseudo code and descriptions

### `main`

Initializes the message module and ncurses and starts the message loop to begin the client/server communication.

```
check for arguments
check if client is player or spectator
initialize display
get hostname, port number and server address from the command line
let server know if client is joining as player or spectator
loop waiting for messages
```

### `sendKeystrokes`

Reads keyboard input from client and sends it to the server.

```
check for any errors with server address
read input from keyboard
if pressed CTRL + C
	exit game
otherwise
	form a message in format 'KEY k'
send the message to the server
```

### `parseMessage`

Receives messages from server and act accordingly.

```
initialize number of rows and columns
initialize player ID

if client receives OK L message
	assign L to player ID

if client receives GRID rows columns message
	assign rows and columns to previously initialized variables
	while true
		get current dimensions of display
		if display dimensions are bigger than rows and columns
			everything good, exit while loop
		otherwise
			tell client to resize the window

if client receives message GOLD n p r
	if client is a player
		display game status for a player
	if client is a spectator
		display game status for a spectator

if client receives message DISPLAY string (map)
	display the game map

if client receives message QUIT summary
	display the quit summary
	exit game

if client receives message ERROR explanation
	display the error explanation
	continue the game
```

### Game
Game stores any information the server might need to know about the game state, such as a hashtable of players gettable by string address, the original map, the current map, and the state of the gold in the game, and the optional spectator.
```c
typedef struct {
  hashtable_t* players;
  grid_t* grid;
  char* mapOG;
  char* mapCurr;
  int remainingGold;
  int remainingPiles;
  int numPlayers;
  player_t* spectator;
} game_t;
```

------------------------------------------------------


## Game module, including player helper functions

### Data structures
The game module uses the player, game, and grid data structures as defined above in the data structures section

### Definition of function prototypes
The following are the functions defined in the game module:

```c
game_t* game_new(char* mapPath);
bool game_addPlayer(game_t* game, char* name, addr_t address);
player_t* game_getPlayer(game_t* game, char* addressStr);
bool game_deletePlayer(game_t* game, char* addressStr);
bool game_addSpectator(game_t* game, addr_t address);
bool game_deleteSpectator(game_t* game);
bool game_move(game_t* game, char* addressStr, int cx, int cy);
void game_sendDisplays(game_t* game);
void game_updateVisibility(game_t* game);
void game_delete(game_t* game);
void game_end(game_t* game);
```

Additionally, within the game module there are some local functions to handle initializing and deleting the player data structure, which listed below and described in the following section as well:

```c
static player_t* player_new(game_t* game, int x, int y, 
				char icon, char* name, addr_t address, bool isSpectator);
static void player_delete(void* item);
```

### Detailed pseudo code and descriptions

#### `game_new`
Initializes the game data structure from the map text file, including determining the map height and width

	open map file
	create game structure
	initialize game player hashtable
	initialize game mapOG
	initialize height and width as 0

	while line in map fp in not NULL
		add the line to mapOG
		get the width if not gotten yet
		increment the height counter
	
	set game gold, piles, numPlayers and spectator
	randomize piles onto map
	close the map fp

#### `game_addPlayer`
Initializes a new player data structure and adds it to the game

	if game is at capacity
		send QUIT message to player
		return false

	copy player name to a new string of maxNameLength
	for every character in the player name
		if it isgraph or is blank
			replace character with '_'
	
	generate player icon
	initialize char
	while char is not '.'
		generate a random x and y in the griod
		set char to the char at x and y
	put the player's icon at the generated x, y

	call player_new with the game, x, y, icon, name, and address
	send OK message to player with icon
	send GRID mesage to player
	send GOLD message to player
	add player struct to the game player hashtable
	increment the number of players
	call game_updateVisibility and game_sendDisplays
	return true

#### `game_getPlayer`
Allows called to get a player given a client address that's been converted to a string. Acts as a wrapper around hashtable_find for the player hashtable in the game struct

#### `game_deletePlayer`
Renders the specified player inactive in the game and updates the game state

	get the player from the address string
	if player is NULL
		return false
	replace the character at player's x, y with '.'
	if couldn't replace the character
		return false
	set player isActive to false
	decrement the game number of players
	call game_updateVisibility
	return true

#### `game_addSpectator`
Adds a specific type of player that is a spectator to the game, checking to see whether a specator already exists and handling if so

	if the game already has a spectator
		get this old spectator and its address
		send QUIT to spectator we are booting
		call player_delete on booted spectator

	intiialize new spectator with player_new
	set game's spectator var to be the new spectator
	send GRID message to spectator
	send GOLD message to spectator
	update the game's visibility

#### `game_deleteSpectator`
Handles deleting the spectator if the spectator quits from the game

	if the game's spectator isn't NULL
		set game's spectator to NULL
		call player_delete on the spectator

#### `game_move`
Handles moving a player by moving it's icon and checking if it has collected gold

	get player from the string adress
	get the char in the currmap at the spot the player wants to move to
	get the char in the OGmap at the player's current spot
	if spot to move to is invalid
		return false
	if spot to move to has fold
		call getGold helper
		send GOLD message to player
	put the player's icon at the spot to move to
	replace the players previous spot with its OGmap cahr
	if any putChar call fails
		return false
	update player x and y
	call game_updateVisbility
	return true

The gold handling aspect of moving a player is handled by the helper function `getGold`, which allocates
a random amount of gold given the amount of gold remaining and piles remaining, and then updates the player's gold and the game state's gold.

#### `game_sendDisplays`
Handles sending DISPLAY messages to all players and spectator by calling `hashtable_iterate` on the player hashtable with the sendDisplay_helper. The pseudocode for sendDisplay_helper is as follows:

	if the player is active
		initialize space for the displayMsg and display
		for int y = 0 that is less than height
			malloc space for the line
			for int x = 0 that is less than width
				set line[x] to the player's visibleMap at the position (y*width)+x
			add \n to end of line
			add \0 to end of line, in response to a valgrind error
			add line to display
			free line
		add display to displayMsg
		send DISPLAY message to player
		free display and displayMsg

#### `game_updateVisibility`
Handles updating the visibleMap of each player and spectator sending DISPLAY messages to all players and spectator by calling `hashtable_iterate` on the player hashtable with the updateVisbility_helper. The pseudocode for updateVisibility_helper is as follows:

	if the player is active
		call grid_addVisiblePoints for the player's information

The description of grid_addVisiblePoints can be found below in the description of the grid module

#### `game_delete`
Cleans up the game data structure by calling `hashtable_delete` on the player hashtable, `grid_delete` on the game grid, and freeing the maps and game

#### `game_end`
Iterates over the players to build a game over summary and then send the summary to all the players and spectators. Deletes the game

	initialize summary message
	call hashtable_iterate(player hashtable, pointer to summary message, getPlayerSummary)
	call hashtable_iterate(player hashtable, summary message, sendPlayerSummary)
	if game's spectator is not null
		send summary to spectator
	call game_delete

The getPlayerSummary helper function dynamically reallocs the summary in order to add a new line for each player in the hashtable containing the player's icon, gold score, and name

The sendPlayerSummary helper function calls `message_send` with the summary for each player in the hashtable

#### `player_new`
Initializes and returns a new player given all the player struct variable info

	intialize new player struct
	set gold to 0
	set x, y, icon, name, address, isSpectator to input parameters
	set isActive to true
	initialize space for visibleMap
	if new player is a spectator
		copy game's mapCurr to visibleMap
	else
		set visibleMap to blank

#### `player_delete`
Handles cleaning up a player struct by freeing the visibleMap and the player struct itself

----

## Grid module

### Data structures
Grid stores the height and width of the map, which is calculated in the game.c module.
```c
typedef struct grid {
  int height;
  int width;
} grid_t;
```
### Definition of function prototypes
The following functions are defined within the grid module:
```c
grid_t* grid_new(int height, int width);
char grid_getChar(grid_t* grid, char* map, int x, int y);
bool grid_putChar(grid_t* grid, char* map, int x, int y, char c);
void grid_display(grid_t* grid, char* map);
void grid_delete(grid_t* grid);
void grid_addVisiblePoints(grid_t *grid, char *mapOG, char* mapCurr, char *visibleMap, int 	px, int py);
static bool tunnel_visibility_helper(grid_t* grid, char* mapOG, char* visibleMap, int px, int py);
bool isVisiblePoint(grid_t* grid, char* mapOG , int x, int y, int px, int py);
static bool is_seeThrough(char c1, char c2);
```
### Detailed pseudo code

#### `grid_new`
Creates a grid structure.
Function receives height and width of grid and returns a pointer to the new grid, or returns NULL if error.
```c
If height is less than or equal to zero or width is less than or equal to zero:
    Return NULL

Allocate memory for a new grid
If the memory allocation fails:
	Return NULL

Set the grid's height to the given height
Set the grid's width to the given width

Return the new grid
```

#### `grid_getChar`
Function gets a character from a location of a map. Function receives grid strucure, map string, location (x, y). Function in the end returns the char at the (x, y) point.
```c
if x or y are out of the grid's range
	return false
else
	get correct char from the map string
```

#### `grid_putChar`
Function puts a character at a certain location in a map 
Function receives grid strucure, map string, location (x, y). Function then returns if char is placed at that point.
```c
if x or y are out of the grid's range
	return false
else
	put char at the correct spot in the string map
```

#### `grid_display`
Function displays the map onto the size of the grid. Function receives grid strucure, map string. Function then puts map string onto grid dimensions.
```c
loop through the height of the map
	for each char in the row
		print the char
	print a new line (after each row)

```

#### `grid_delete`
Function deletes the grid by freeing the pointers. Function receives grid strucure and frees that grid structure.
```c
free the grid
```

#### `grid_addVisiblePoints`
Function adds visible points to visibleMap string (for a given player)

Function receives grid, current map, visible map (where the visible map string goes), and current x and y for player. Sets visibleMap to the correct string for respective player.
```c
check if the player is in a room/ a tunnel spot where they can see into a room
    for all x
        for all y
            get the chars from current map, user's visible map, and original map
            if (x,y) is visible from player,
                put current map char on visible map
            if (x,y) is not visible, but a previously seen player or gold
                put og map char on visible map
if in tunnel
    for all x
        for all y
            fill all visible spots of map with og map

put an @ at current location
```

#### `tunnel_visibility_helper`
Returns boolean value if is the entrance to a room
```c
loops through all points within 1 space from spot
    update map with all visible chars
    if one of the spots is a room space
        mark isEntrance true
return isEntrance
```

#### `isVisiblePoint`
Returns a boolean if a point (x,y) is visible from a players (px, py) on the og map
```c
if x,y = px, py
    return true
if x,y is an emtpy char
    return false

if x,y and px,py exist on vertical line
    loop for each point on the line
        if non room spot on og map
            return false
    return true

find slope
loop for each point on the line
    if point falls between two grid points
        if not is_seeThrough
            return false
    if point is a grid point
        if it is not a open room spot
            return false
return true
```

#### `is_seeThrough`
Returns a boolean if two points are see through (as in at most one is a wall space and the other is room space)
```c
if either space is blank
    return false
if both spots are walls
    return false
else
    return true
```

--- 


## Server

### Data structures

Server makes use of the `game` and `player` structures, described above. 

### Definition of function prototypes

The following are the functions used within server:
```c
static void parseArgs(const int argc, char* argv[], game_t** game);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static bool handleKeypress(addr_t from, char key, game_t* game);
```

### Detailed pseudo code and function descriptions

#### `main`
The `main` function calls `parseArgs` and `message_init` to initialize the game and the port for receiving messages. It then calls `message_loop` with the handleMessage helper to listen for messages from clients.

#### `parseArgs`
Parse args handles parsing the map text file and optional seed from the command line. If the seed is not provided, it initializes `srand()` with the time as a random seed. It also calls `game_new` on the map text file, which checks the text fle and initializes the game. Parseargs runs as follows

	validate number of commandline args
	if there are 3 arguments
		convert string seed to integer, validate
		seed the random-number generator with that seed
	else
		seed the random-number generator with time
	initialize game_new from the textfile
	assign to the new game to the pointer argument

#### `handleMessage`
The helper function used by message_loop to handle incoming messages. It checks which type of message it is receiving from the client - if the message is for a new player or spectator, it handles it via calls to game, and if the message is a key it calls `handleKeypress` to parse the key

	copy message into a non const char* to loop through and edit
	assign the messageCpy pointer to be the code
	while the char at *messageCpy is not a space or null character
		increment the character pointer
	if the character at the pointer is a space
		set it to null and increment
	assign the pointer to be the params

	if the code is PLAY
		copy the player name from params
		call game_addPlayer
	if the code is SPECTATOR
		call game_addSpectator
	if the code is KEY
		get the key from params
		call handleKeypress
	else
		invalid key, log to stderr
	free the messageCpy

#### `handleKeypress`
A helper function to the handleMessage function, which identifies which key was passed by the client and calls the appropriate action (end game, move, etc)

	get a copy of the string version of the address (passed to move later)
	If the key is Q
		if there's a spectator and it's address matches the message
			send QUIT to spectator
			delete spectator
		else 
			call game_deletePlayer with the address string
			if deleting a player is sucessful
				send QUIT to player
	if the key is lowercase
		call game_move with the address copy and the correct direction for the key
	if the key is uppercase
		call a while loop of game_move with the address copy and the correct direction for the key
	send updated displays
	if the remaining gold in the game is now 0
		call end_game
		return true
	return false (keep looping)
	
## Testing plan

### unit testing

> How we tested each unit (module) before integrating them with a main program (client or server):
 
 We tested grid as a standalone with gridtest, which tested all the functions of grid that didn't need game functionality. Next, we focused on testing the game module in isolation, checking all aspects that didn’t require client-server interactions. Afterward, we moved on to testing the game with server functionality but without involving the client, verifying that message sending worked correctly.


### integration testing

> How we tested the complete main programs: the server, and for teams of 4, the client:

To test the server and client, we started by checking individual parts of the code, such as message handling and grid functions. Then, we ran the server and client together to verify their communication and gameplay handling. Finally, we simulated real gameplay to ensure everything worked smoothly, using tools like Valgrind and GDB to catch any errors. 

### system testing

> How we you tested our client and server together:

We tested game across a variety of scenarios, such as edge cases and typical cases. These tests focused on validating proper communication between the client and server, ensuring expected gameplay behavior, and identifying incorrect functionality with valgrind or gdb when necessary.
---

## Limitations

> None...
