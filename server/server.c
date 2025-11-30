/* 
 * server - the primary server executable for nuggets which establishes a port, handles client messages, 
 *  and makes calls to the game interface to change the game state
 *
 * JL3, CS50 Final Project 11/14/24
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "game.h"
#include "message.h"
#include "log.h"

//********************* prototypes *********************
static void parseArgs(const int argc, char* argv[], game_t** game);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static bool handleKeypress(addr_t from, char key, game_t* game);

//********************* main *********************

int
main(int argc, char* argv[]) 
{
  game_t* game = NULL;
  log_init(stderr);
  parseArgs(argc, argv, &game);

  int port = message_init(stderr);
  fprintf(stdout, "Server initialized, waiting at port %d\n", port);

  if (port == 0){ // error initializing server
    log_e("Error: message_init failed to assign port\n");
    return 4;
  } else {
    bool gameOn;
    gameOn = message_loop(game, 0, NULL, NULL, handleMessage);
    if (!gameOn){
      game_end(game);
      return 5;
    }
    return 0;
  }
}

/**************** parseArgs ****************/
/* 
 * Handles command line arguments, sets random seed,
 * and initializes new game
 * 
 * Caller provides:
 *   argc and argv[] from main
 *   a pointer to a null game_t* to initialize
 * 
 * We exit non-zero if any errors are encountered,
 * logging to stderr as well
 */
static void
parseArgs(const int argc, char* argv[], game_t** game)
{
  if (argc == 1 || argc > 3){ // incorrect number of arg
    log_e("Usage: ./server map.txt [seed]\n");
    exit(1);
  }
  if (argc == 3){ // yes optional seed arg
    // handle seed arg
    char* seedStr = argv[2];
    int seed;
    if (sscanf(seedStr, "%d", &seed) != 1){
      log_e("Error: invalid seed argument, not an int\n");
      exit(2);
    }
    srand(seed);
  } else { // no seed, generate randomly
    srand(time(NULL));
  }

  // handle map.txt arg: assign to game pointer
  char* filename = argv[1];
  *game = game_new(filename);
  if (*game == NULL){
    log_e("Error: could not initialize game\n");
    exit(3);
  }
}

/**************** handleMessage ****************/
/* 
 * Handles message obtained by the messageLoop function,
 * calling the appropriate game methods based on the message
 *
 * Caller (messageLoop) provides:
 *   arg, containing a pointer to the game
 *   the address the message was sent from
 *   the message itself
 * 
 * We return true if the inputs are valid and the appropriate
 * action was able to be executed, false otherwise with
 * logging to stderr as well
 */
static bool
handleMessage(void* arg, const addr_t from, const char* message) //check how message memory is handled
{
  bool gameOver = false;
  game_t* game = arg;
  if (message == NULL || game == NULL){
    return true;  //end looping -not sure if this logic makes sense
  }

  // get code from message by splitting string on space
  char* messageCpy = malloc(strlen(message) + 1); //message copy for tokenization
  if (messageCpy == NULL) {
    return true; //end looping
  }
  strcpy(messageCpy, message);

  // split messageCpy into code and params
  char* code = messageCpy;
  char* messagePos = messageCpy;
  while (*messagePos != ' ' && *messagePos != '\0') {
    messagePos++;
  }
  if (*messagePos == ' ') {
    *messagePos = '\0';
    messagePos++;
  }
  char* params = messagePos; // points to the rest of the message

  if (strcmp(code, "PLAY") == 0 && game != NULL){
    char name[strlen(params)+1];
    strcpy(name, params);
    game_addPlayer(game, name, from);
    // add some kind of error handling if addplayer doesn't work

  } else if (strcmp(code, "SPECTATE") == 0){
    game_addSpectator(game, from);
    // add some kind of error handling if addspec doesn't work

  } else if (strcmp(code, "KEY") == 0){
    if (strlen(params) == 1){
      char key = *params;
      gameOver = handleKeypress(from, key, game); //returns true if game over, false otherwise
    }
  } else { // not correct message type
    log_e("Error: message from client not PLAY, SPECTATE, or KEY\n");
  }

  if (game == NULL) {
    game_sendDisplays(game);
  }
  free(messageCpy);
  return gameOver;   
}


/**************** handleKeyPress ****************/
/* 
 * Handles calling move or game_deletePlayer based on key
 * sent from client
 *
 * Caller (messageLoop) provides:
 *   the address the message was sent from
 *   the message itself
 *   the game data structure
 * 
 * We return true if the keyPress causes the game to end,
 * else we return false. There is logging to stderr
 * addCpy must be freed after it is used outside of handleKeypress
 */
static bool
handleKeypress(addr_t from, char key, game_t* game)
{
  const char* add = message_stringAddr(from);
  char addCpy[strlen(add)+1];
  strcpy(addCpy, add);

  // handle only possible spectator action first
  addr_t* spectatorAddress = game_getSpectatorAddress(game);
  if (key == 'Q' && spectatorAddress != NULL && message_eqAddr(from, *spectatorAddress)){
    game_deleteSpectator(game);
  }
  else if (spectatorAddress == NULL || (spectatorAddress != NULL && !message_eqAddr(from, *spectatorAddress)))
  { // otherwise handle player actions, as long as they aren't from the spectator
    switch (key) {
      case 'Q': game_deletePlayer(game, addCpy);
      case 'h': game_move(game, addCpy, -1, 0); break;
      case 'l': game_move(game, addCpy, 1, 0); break;
      case 'k': game_move(game, addCpy, 0, -1); break;
      case 'j': game_move(game, addCpy, 0, 1); break;
      case 'y': game_move(game, addCpy, -1, -1); break;
      case 'u': game_move(game, addCpy, 1, -1); break;
      case 'b': game_move(game, addCpy, -1, 1); break;
      case 'n': game_move(game, addCpy, 1, 1); break;

      case 'H': while(game_move(game, addCpy, -1, 0)); break;
      case 'L': while(game_move(game, addCpy, 1, 0)); break;
      case 'K': while(game_move(game, addCpy, 0, -1)); break;
      case 'J': while(game_move(game, addCpy, 0, 1)); break;
      case 'Y': while(game_move(game, addCpy, -1, -1)); break;
      case 'U': while(game_move(game, addCpy, 1, -1)); break;
      case 'B': while(game_move(game, addCpy, -1, 1)); break;
      case 'N': while(game_move(game, addCpy, 1, 1)); break;
      default: log_e("Error: KEY message from client has invalid key\n"); break;
    }
  }
  game_sendDisplays(game);

  // check if game has ended
  if (game_getRemainingGold(game) == 0) {
    game_end(game);
    return true;
  }
  return false;
}

