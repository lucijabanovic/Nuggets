/* 
 * game.h - header file for nugget's game module
 *
 * JL3, CS 50, Fall 2024
 *
 */

#ifndef __GAME_H
#define __GAME_H

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


/**************** global types ****************/
typedef struct game game_t;



/**************** game_new ****************/
/* 
 * Creates a new instance of the game structure
 *
 * Caller provides:
 *   Valid string to map path
 * We return:
 *   Game, a struct which contains info about the
 *   game state, or NULL if something errored during making
 * Caller is responsible for:
 *  Later calling game_delete on the game
 * Notes: 
 *   assumes the random seed has already been set
 */
game_t* game_new(char* mapPath);

/**************** game_addPlayer ****************/
/* 
 * Adds a player to game structure 
 *
 * Caller provides:
 *   Game
 *   Name of player
 *   Address of player
 * We return:
 *  True, if the player was successfully added
 *  False, if game is full or error in adding occured
 * Notes:
 *   Players are added to a hashtable and freed from this
 *   hashtable in game_delete
 */
bool game_addPlayer(game_t* game, char* name, addr_t address);

/**************** game_deletePlayer ****************/
/* 
 * "deletes" a player from the game structure by inactivating
 * the player and resetting its icon in the map. All players
 * are later deleted (as in freed) from the ht in game_delete
 *
 * Caller provides:
 *   Game
 *   String address of the player
 * We return:
 *  True, if the player was successfully deleted
 *  False, if error in deletion occured
 */
bool game_deletePlayer(game_t* game, char* addressStr);

/**************** game_addSpectator ****************/
/* 
 * Adds a spectator to game structure 
 *
 * Caller provides:
 *   Game
 *   Address of spectator
 * We return:
 *  True, if the spec was successfully added
 *  False, if game is full or error in adding occured
 */
bool game_addSpectator(game_t* game, addr_t address);

/**************** game_getSpectatorAddress ****************/
/* 
 * Returns the address of the spectator, if one 
 *
 * Caller provides:
 *   Game
 * We return:
 *  Address, if the spectator exists
 *  Null, if no spectator 
 */
addr_t* game_getSpectatorAddress(game_t* game);
/**************** game_deleteSpectator ****************/
/* 
 * Deletes a spectator from the game structure 
 *
 * Caller provides:
 *   Game
 * We return:
 *  True, if the spectator was successfully deleted 
 *  False, if error in deletion occured
 */
bool game_deleteSpectator(game_t* game);

/**************** game_move ****************/
/* 
 * Moves the specified player by cx in the x axis
 * and cy in the y axis if possible
 *
 * Caller provides:
 *   Game
 *   String address of the player
 *   Distance to move in x
 *   Distance to move in y
 * We return:
 *  True, if the move was executed 
 *  False, if the move was not possible
 */
bool game_move(game_t* game, char* addressStr, int cx, int cy);

/**************** game_sendDisplays ****************/
/* 
 * Sends an updated display message to every player in 
 * the game
 *
 * Caller provides:
 *   Game
 * We return void
 */
void game_sendDisplays(game_t* game);

/**************** game_updateVisibility ****************/
/* 
 * Updates every player in the game's visible map as well as 
 * the spectators map to be the most current
 *
 * Caller provides:
 *   Game
 */
void game_updateVisibility(game_t* game);

/**************** game_getRemainingGold ****************/
/* 
 * Getter method for the int remaining gold
 *
 * Caller provides:
 *   Game
 */
int game_getRemainingGold(game_t* game);
/**************** game_delete ****************/
/* 
 * Cleans up the game data structure, including the
 * player ht, grid, and maps
 *
 * Caller provides:
 *  Game
 */
void game_delete(game_t* game);

/**************** game_end ****************/
/* 
 * Generates and sends game summary to all players
 * and spectator, calls game_delete to clean up game
 *
 * Caller provides:
 *  Game
 */
void game_end(game_t* game);

#endif // __GAME_H
