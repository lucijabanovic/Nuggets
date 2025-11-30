/*
 *
 * game.c - game structure for nuggets project 
 *
 * JL3, CS 50, Fall 2024
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include "hashtable.h"
#include "message.h"
#include "grid.h"
#include "mem.h"
#include "file.h"
#include "game.h"

//GAME ISSUES LIST:
// - spectator doesn't get sent display
// - Shift moving doesn't work
// - server doesn't quit client if it fails


/**************** local constants **************/
static const int maxNameLength = 50;   // max number of chars in playerName
static const int maxPlayers = 26;      // maximum number of players
static const int goldTotal = 250;      // amount of gold in the game
static const int goldMinPiles = 10; // minimum number of gold piles
static const int goldMaxPiles = 20; // maximum number of gold piles
static const int messageMaxBytes = 65507;  // max message/map length
static const char goldSpot = '*';
static const char wallSpotVert = '|';
static const char wallSpotHoriz = '-';
static const char wallSpotCorn = '+';
static const char blank = ' ';

/**************** local types ****************/
typedef struct player {
  int gold;         // player's purse
  int x;            // player x coord
  int y;            // player y coord
  char icon;        // player char in map
  char* name;       // player string name
  addr_t address;   // player client address
  char* visibleMap;   // map of what player can see
  bool isSpectator;   // is player a spectator
  bool isActive;    // has player qut
} player_t;

/**************** global types ****************/
typedef struct game {
  hashtable_t* players;  // all players in game by string address
  hashtable_t* icons;    // all players in game by icon
  grid_t* grid;          //  map parameters
  char* mapOG;           // unaltered map
  char* mapCurr;         // current map state
  int remainingGold;     // amount of gold left
  int remainingPiles;    // ammount of piles left
  int numPlayers;        // num of players
  player_t* spectator;   // the game's spectator
} game_t;

/**************** global functions ****************/
/* that is, visible outside this file */
/* see game.h for comments about exported functions */


/**************** local functions ****************/
/* not visible outside this file */
static player_t* player_new(game_t* game, int x, int y, char icon, char* name, addr_t address, bool isSpectator);
static player_t* player_get(game_t* game, char* address);
static player_t* player_getFromIcon(game_t* game, char icon);
static void player_swap(game_t* game, player_t* player1, player_t* player2);
static void player_delete(void* item);
static void randomizePileLocations(game_t* game);
static void getPlayerSummary(void *arg, const char *key, void *item);
static void sendPlayerSummary(void* arg, const char* key, void* item);
static void getGold(game_t* game, player_t* player);
static void sendDisplay_helper(void* arg, const char* key, void* item);
static void visibility_helper(void* arg, const char* key, void* item);


/**************************** game module functions **************************/

/**************** game_new ****************/
/* see game.h for details */
game_t*
game_new(char* mapPath)
{
  //open map and check its readable
  FILE* fp = fopen(mapPath, "r");
  if (fp == NULL){
    return NULL;
  }

  //create game struct
  game_t* game = mem_malloc(sizeof(game_t));
  if (game == NULL){
    return NULL;
  }

  //initialize players hashtable
  game->players = hashtable_new(maxPlayers);
  if (game->players == NULL){
    mem_free(game);
    return NULL;
  }

  game->icons = hashtable_new(maxPlayers);
  if (game->icons == NULL){
    mem_free(game);
    return NULL;
  }


  //malloc space for map
  char *mapOG = malloc(messageMaxBytes);
  mapOG[0] = '\0';   //added so can use strcat with blank string

  //figure out map size for grid structure
  int height = 0;
  int width = 0;

  char* line;
  while ((line = file_readLine(fp)) != NULL){
    strcat(mapOG, line);
    if (height == 0){
      width = strlen(mapOG);
    }
    mem_free(line);
    height++;
  }
  
  // set up player maps and grid
  char* mapCurr = malloc(messageMaxBytes);
  strcpy(mapCurr, mapOG);
  game->mapCurr = mapCurr;
  game->mapOG = mapOG;
  game->grid = grid_new(height, width);

  // set gold, spectator and numPlayers
  game->remainingGold = goldTotal;
  game->remainingPiles = (rand() % (goldMaxPiles - goldMinPiles)) + goldMinPiles;
  game->numPlayers = 0;
  game->spectator = NULL;

  // add gold to map
  randomizePileLocations(game);

  fclose(fp);
  return game;
}

/**************** game_addPlayer ****************/
/* see game.h for details */
bool
game_addPlayer(game_t* game, char* name, addr_t address)
{
  if (game == NULL || name == NULL || strlen(name) < 1){
    return false;
  }
  
  // check game not at capacity
  if (game->numPlayers >= maxPlayers){
    message_send(address, "QUIT game is at max player capacity");
    return false;
  }
  
  // truncate player name if needed 
  char* shortenedName = malloc(sizeof(char)*(maxNameLength+1));
  strncpy(shortenedName, name, maxNameLength+1);
  shortenedName[maxNameLength] = '\0';
  for (int i = 0; i < strlen(shortenedName); i++){
    if (isgraph(shortenedName[i]) == 0 && isblank(shortenedName[i]) == 0){
      shortenedName[i] = '_';
    }
  }
  

  // initialize player
  int x = -1;   
  int y = -1;
  char icon = 'A' + game->numPlayers; 
  char c = wallSpotVert;
  // find a empty room spot
  while (c != '.'){
    x = rand() % grid_getWidth(game->grid);
    y = rand() % grid_getHeight(game->grid);
    c = grid_getChar(game->grid, game->mapCurr, x, y);
  }
  //put the player there
  grid_putChar(game->grid, game->mapCurr, x, y, icon);

  //create the player struct
  player_t* player = player_new(game, x, y, icon, shortenedName, address, false);
  if (player == NULL){
    free(shortenedName);
    return false;
  } 

  // send confirm message to client
  char okMsg[5];
  sprintf(okMsg, "OK %c", icon);
  message_send(address, okMsg);

  // send grid message to client
  char gridMsg[15];
  sprintf(gridMsg, "GRID %d %d", grid_getHeight(game->grid), grid_getWidth(game->grid));
  message_send(address, gridMsg);

  // send gold message
  char goldMsg[20];
  sprintf(goldMsg, "GOLD %d %d %d", 0, player->gold, game->remainingGold);
  message_send(address, goldMsg);
  
  // add player to hashtable
  if (hashtable_insert(game->players, message_stringAddr(address), player) == false){
    free(shortenedName);
    return false;
  }

  // add player to hashtable
  char str[2];
  str[0] = icon;
  str[1] = '\0';
  if (hashtable_insert(game->icons, str, player) == false){
    free(shortenedName);
    return false;
  }


  game->numPlayers++;

  //update other players
  game_updateVisibility(game); 
  game_sendDisplays(game);

  return true; 
}

/**************** game_deletePlayer ****************/
/* see game.h for details */
bool
game_deletePlayer(game_t* game, char* addressStr)
{
  if (game == NULL){
    return false;
  }

  // get player by address and free address
  player_t* player = player_get(game, addressStr);

  if (player == NULL){
    return false;
  }

  // make player inactive
  player->isActive = false;
  message_send(player->address, "QUIT Thanks for playing!");
  return true; 
}

/**************** game_addSpectator ****************/
/* see game.h for details */
bool
game_addSpectator(game_t* game, addr_t address)
{
  if (game == NULL){
    return false;
  }

  // handle case where game already has spectator
  if (game->spectator != NULL){
    player_t* prevSpectator = game->spectator;
    addr_t spectatorAddr = prevSpectator->address;
    message_send(spectatorAddr, "QUIT You have been replaced by a new spectator");
    player_delete(prevSpectator);
  }

  player_t* spectator = player_new(game, 0, 0, '\0', "", address, true);

  // malloc space for spectator visible map
  if (spectator == NULL){
    return false;
  } 
  game->spectator = spectator; //not stored in hashtable with other players
  
  // send grid message
  char gridMsg[15]; // should there be any basis to this size?
  sprintf(gridMsg, "GRID %d %d", grid_getHeight(game->grid), grid_getWidth(game->grid));
  message_send(address, gridMsg);

  // send gold message
  char goldMsg[20]; //also should there be any basis to this?
  sprintf(goldMsg, "GOLD %d %d %d", 0, 0, game->remainingGold);
  message_send(address, goldMsg);

  // get the most current map for the spectator
  game_updateVisibility(game);
  game_sendDisplays(game);

  return true;
}

/**************** game_getSpectatorAddress ****************/
/* see game.h for details */
addr_t*
game_getSpectatorAddress(game_t* game)
{
  if (game->spectator == NULL){
    return NULL;
  }
  return &(game->spectator->address);
}

/**************** game_deleteSpectator ****************/
/* see game.h for details */
bool
game_deleteSpectator(game_t* game)
{
  if (game == NULL){
    return false;
  }
  if (game->spectator == NULL){
    return false;
  }
  // send quit message
  message_send(game->spectator->address, "QUIT Thanks for watching!");

  // free the spectator and set to null
  player_delete(game->spectator);
  game->spectator = NULL;
  

  return true; 
}

/**************** game_move****************/
/* see game.h for details */
bool 
game_move(game_t* game, char* addressStr, int cx, int cy)
{
  player_t* player = player_get(game, addressStr);

  char spot = grid_getChar(game->grid, game->mapCurr, player->x + cx, player->y + cy);
  char oldSpot = grid_getChar(game->grid, game->mapOG, player->x, player->y);

  if (spot == wallSpotHoriz || spot == wallSpotCorn || spot == wallSpotVert || spot == blank){
    //printf("can't move there (game.c in move)"); i don't think we are meant to be printing at all? could log_e
    return false;
  }

  if (spot == goldSpot){
    getGold(game, player);
    char goldMsg[20]; //also should there be any basis to this?
    sprintf(goldMsg, "GOLD %d %d %d", 0, player->gold, game->remainingGold);
    message_send(player->address, goldMsg);

  }

  // if trying to swap into another player
  if (isalpha(spot)){
    // swap positions
    player_t* player2 = player_getFromIcon(game, spot);
    player_swap(game, player, player2); 
  } else {
    // move the player and place old item back at spot
    grid_putChar(game->grid, game->mapCurr, player->x + cx, player->y + cy, player->icon);
    grid_putChar(game->grid, game->mapCurr, player->x, player->y, oldSpot);
    player->x = player->x + cx;
    player->y = player->y + cy;
  }

  // update game
  game_updateVisibility(game);
  game_sendDisplays(game);
  return true;
}



/**************** game_sendDisplays ****************/
/* see game.h for details */
void
game_sendDisplays(game_t* game)
{
  hashtable_iterate(game->players, game, sendDisplay_helper);
  if (game->spectator != NULL){
    sendDisplay_helper(game, "key", game->spectator);
  }
}

/**************** game_updateVisibility ****************/
/* see game.h for details */
void
game_updateVisibility(game_t* game)
{
  hashtable_iterate(game->players, game, visibility_helper);
  if (game->spectator != NULL){
    strcpy(game->spectator->visibleMap, game->mapCurr);
  }
}

/**************** game_getRemainingGold ****************/
/* see game.h for details */
int
game_getRemainingGold(game_t* game)
{
  if (game == NULL) // for game delete checking post factum
  {
    return 0; 
  }
  return game->remainingGold;
}

/**************** game_delete ****************/
/* see game.h for details */
void
game_delete(game_t* game)
{
  // delete hashtable with players
  hashtable_delete(game->players, player_delete);

  // delete spectator if exists
  if (game->spectator != NULL){
    player_delete(game->spectator);
  }

  // delete hashtable with icons (players already deleted)
  hashtable_delete(game->icons, NULL);

  // delete grid
  grid_delete(game->grid);

  // free the maps
  mem_free(game->mapOG);
  mem_free(game->mapCurr);

  mem_free(game);
  game = NULL;
}

/**************** game_end****************/
/* see game.h for details */
void
game_end(game_t* game)
{
  char* message = "QUIT GAME OVER:\n\r";
  char* summary = malloc(strlen(message)+(maxNameLength+15)*game->numPlayers+1);
  strcpy(summary, message);

  // build player summary
  hashtable_iterate(game->players, &summary, getPlayerSummary);

  // send player summary
  hashtable_iterate(game->players, summary, sendPlayerSummary);

  // send summary to spectator
  if (game->spectator!=NULL) {
    message_send(game->spectator->address, summary);
  }
  free(summary);
  game_delete(game);
}
/**************************** player module functions **************************/

/**************** player_new ****************/
/* 
 * Creates a new player structure
 *
 * Notes:
 *  Can create a player or spectator
 *  Calls updateVisibility to create user specific map
 *  Players are turned inactive by game_deletePlayer, but freed
 *  in game_delete
 */
static player_t*
player_new(game_t* game, int x, int y, char icon, char* name, addr_t address, bool isSpectator)
{
  // initialize player
  player_t* player = malloc(sizeof(player_t));
  if (player == NULL){
    return NULL;
  }

  // set player attributes
  player->gold = 0;
  player->x = x;
  player->y = y;
  player->icon = icon;
  player->name = name;
  player->address = address;
  player->isActive = true;
  player->isSpectator = isSpectator;

  // malloc space for visible map
  player->visibleMap = malloc(messageMaxBytes);
  if (player->visibleMap == NULL){
    return NULL;
  }

  // set up player map
  if (isSpectator){
    strcpy(player->visibleMap, game->mapCurr);
  } else {
    memset(player->visibleMap, ' ', 9998);
    player->visibleMap[9999] = '\0';
  }
  
  return player;
}

/**************** player_get ****************/
/*
 * return a player based on their addressStr
 *
 */
static player_t*
player_get(game_t* game, char* addressStr)
{
  player_t* playerMatch = hashtable_find(game->players, addressStr);
  return playerMatch;
}

/**************** player_getFromIcon ****************/
/*
 * return a player based on their icon 
 *
 */
static player_t*
player_getFromIcon(game_t* game, char icon)
{
  // convert icon into string
  char str[2];
  str[0] = icon;
  str[1] = '\0';

  player_t* playerMatch = hashtable_find(game->icons, str);
  return playerMatch;
}

/**************** player_delete ****************/
/* 
 * swap the location of two players
 */
static void
player_swap(game_t* game, player_t* player1, player_t* player2)
{
  // move the players to each other's spot
  grid_putChar(game->grid, game->mapCurr, player2->x, player2->y, player1->icon);
  grid_putChar(game->grid, game->mapCurr, player1->x, player1->y, player2->icon);

  // swap coordinates
  int tempX = player1->x;
  int tempY = player1->y;
  player1->x = player2->x;
  player1->y = player2->y;
  player2->x = tempX;
  player2->y = tempY;

}

/**************** player_delete ****************/
/* 
 * Deletes a player structure
 *
 * Notes:
 *  Used in game_delete when it calls hashtable_delete
 */
static void
player_delete(void* item)
{
  player_t* player = (player_t*) item;
  if (!player->isSpectator){
    mem_free(player->name);
  }
  mem_free(player->visibleMap);
  mem_free(player);
}

/**************************** local helper functions **************************/

/**************** randomizePileLocations ****************/
/* 
 * Disperses piles onto mapCurr of game according to local
 * constants that define the gold piles. Amoutn in pile is not
 * determined until picked up by a player
 * LUKAS add comments
 */
static void
randomizePileLocations(game_t* game)
{
  int x = -1;
  int y = -1;
  char goldIcon = '*';
  // for each needed pile
  for (int i = 0; i < game->remainingPiles; i++){
    char c = wallSpotVert;
    // find a valid empty spot
    while (c != '.'){
      x = rand() % grid_getWidth(game->grid);
      y = rand() % grid_getHeight(game->grid);
      c = grid_getChar(game->grid, game->mapCurr, x, y);
    }
    // put gold in spot
    grid_putChar(game->grid, game->mapCurr, x, y, goldIcon);
  }
}

/**************** visibility_helper ****************/
/* 
 * Add visible points to all active players visible maps 
 *
 */
static void
visibility_helper(void* arg, const char* key, void* item)
{
  game_t* game = arg;
  player_t* player = item;

  if (player->isActive == true){
    grid_addVisiblePoints(game->grid, game->mapOG, game->mapCurr, player->visibleMap, player->x, player->y);
  }
}

/**************** sendDisplay_helper ****************/
/* 
 * send displays to all active users 
 */
static void
sendDisplay_helper(void* arg, const char* key, void* item)
{
  game_t* game = arg;
  player_t* player = item;

  int height = grid_getHeight(game->grid);
  int width = grid_getWidth(game->grid);

  // if the player is active
  if (player->isActive == true){
    char displayMsg[10 + strlen(game->mapOG) + height];

    // malloc space for display with new line chars
    char *display = malloc(10 + strlen(game->mapOG) + height);

    // make it so can cat to display
    display[0] = '\0';
    
    // add new line chars
    for(int y = 0; y < height; y++) {
      char* line = mem_malloc(width + 2);
      line[0] = '\0';
      for (int x = 0; x < width; x++){
        int c = (y * width) + x;
        line[x] = player->visibleMap[c];
      }
      line[width] = '\n';
      line[width + 1] = '\0';
      strcat(display, line);
      free(line);
    }

    // send message
    sprintf(displayMsg, "DISPLAY\n%s", display);
    message_send(player->address, displayMsg);
    free(display);
  }
}

/**************** getPlayerSummary ****************/
/* 
 * hashtable_iterate helper which adds each players information
 * into the summary char** passed as an arg
 */
static void
getPlayerSummary(void* arg, const char* key, void* item)
{
  char** summary = arg;
  player_t* player = (player_t*) item;
  char* name = player->name;
  char icon = player->icon;
  int score = player->gold;

  int summaryLen = strlen(*summary);
  sprintf(*summary + summaryLen, "%c %6d %s\n", icon, score, name);
}

/**************** sendPlayerSummary ****************/
/* 
 * hashtable_iterate helper which sends the game over
 * summary to all players in the game
 */
static void
sendPlayerSummary(void* arg, const char* key, void* item)
{
  char* summary = arg;
  player_t* player = (player_t*) item;
  message_send(player->address, summary);
}

/**************** getGold ****************/
/* 
 * add gold amount to player
 */
static void getGold(game_t* game, player_t* player){
  int gold;
  // if one pile left
  if (game->remainingPiles == 1){
    gold = game->remainingGold;
    player->gold += gold; 
  } else {
    // randomize gold amount around the expected amount they'd find
    gold = (game->remainingGold / game->remainingPiles) + (rand() % 3);
    // if negative, give them 1 gold
    if (gold <= 0){
      gold = 1;
    }
    // add gold to player
    player->gold += gold; 
  }
  // remove necessary gold and piles from game
  game->remainingGold -= gold;
  game->remainingPiles -= 1;
}
