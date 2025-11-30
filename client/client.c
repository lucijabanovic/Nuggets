/*
 * client.c - a client module for the Nuggets game CS50 final project
 * Lucija Banovic, FALL 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
#include "message.h"

/**************** file-local functions ****************/

static bool sendKeystrokes(void *arg);
static bool parseMessage(void *arg, const addr_t from, const char *message);

// global bool to check if the client is player or spectator
bool isPlayer = false;

/***************** main *******************************/
int main(const int argc, char *argv[]) {
    // initialize the message module
    if (message_init(NULL) == 0) {
    	fprintf(stderr, "Error: failure to initialize message module\n");
    	return 1;
    }
    
    // check arguments
    if (argc != 3 && argc != 4) {
    	fprintf(stderr, "Usage: ./client <hostname> <port> [playername]\n");
    	return 1;
    }
    
    // check for player/spectator
    if (argc == 4) {
    	isPlayer = true;
    }
    
    // initialize ncurses display
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    
    // get hostname, port and server address from command line
    const char *host = argv[1];
    const char *port = argv[2];
    addr_t server;
    if (!message_setAddr(host, port, &server)) {
    	fprintf(stderr, "Error: can't form address from %s %s\n", host, port);
    	return 1;
    }

    // first, let server know whether client is joining as a player or a spectator
    if (isPlayer) {
    	char playermsg[20];
    	sprintf(playermsg, "PLAY %s", argv[3]);
	    fprintf(stderr, "Joining as player\n");
    	message_send(server, playermsg);
    } else {
	    fprintf(stderr, "Joining as spectator\n");
    	message_send(server, "SPECTATE");
    }
    
    // Loop, waiting for input or for messages
    // We use the 'arg' parameter to carry a pointer to 'server'.
    bool ok = message_loop(&server, 0, NULL, sendKeystrokes, parseMessage);
    
    // shut down the message module
    message_done();
    fprintf(stderr, "Shutting down message module\n");
    
    return ok ? 0 : 1; // status code depends on result of message_loop
}

/**************** sendKeystrokes ****************/
/* ncurses display reads input from client character by character and sends to server
 * Return true if the message loop should exit, otherwise false.
 * i.e., return true if EOF was encountered (CTRL + C), or fatal error.
 */
static bool sendKeystrokes(void *arg) {
    // We use 'arg' to receive an addr_t referring to the 'server' correspondent.
    // Defensive checks ensure it is not NULL pointer or non-address value.
    addr_t *serverp = arg;
    if (serverp == NULL) {
        fprintf(stderr, "Error: handleInput called with arg=NULL\n");
        return true;
    }
    if (!message_isAddr(*serverp)) {
        fprintf(stderr, "Error: handleInput called without a correspondent.\n");
        return true;
    }

    // ncurses reads character by character
    // we initialize a single int instead of char because ncurses accepts ASCII values
    int ch;
    char message[6];

    // read a line from ncurses display
    // capture until CTRL + C (ASCII 3)
    if ((ch = getch()) == 3) {
        // EOF case: stop looping
	    fprintf(stderr, "Exiting ncurses...\n");
        endwin();
        return true;
    } else {
        // message containing a client keystroke
        sprintf(message, "KEY %c", (char)ch);
    }

    // send as message to server
    fprintf(stderr, "Sending message to server: %s\n", message);
    message_send(*serverp, message);

    // normal case: keep looping
    return false;
}

/**************** parseMessage ****************/
/* Receives message from server (GRID, GOLD, DISPLAY, QUIT, OK, ERROR)
 * Perform an action according to the message
 * Return true if any fatal error
 */
static bool parseMessage(void *arg, const addr_t from, const char *message) {
	// number of rows and number of columns
	static int NR = 0, NC = 0;

	// player ID (e.g. A, B, C etc.)
	static char letter;

	// if client receives a message "OK player id", assign player ID to letter
	if (strncmp(message, "OK", 2) == 0) {
		sscanf(message, "OK %c", &letter);
	}

	// if client receives a message "GRID rows columns", assign those values to NR and NC
	if (strncmp(message, "GRID", 4) == 0) {
		sscanf(message, "GRID %d %d", &NR, &NC);
		
		int rows, cols;
		while (1) {
			// get currect dimensions of ncurses display
			getmaxyx(stdscr, rows, cols);

			// if the dimensions of display are equal or greater than NR and NC, exit while loop
			if (rows >= NR+1 && cols >= NC+1) {
				break;
			}

			// if the dimensions of display are smaller than NR and NC, tell client to resize window
			mvprintw(0, 0, "Resize window for better results\n");
            refresh();
            napms(500);
		}

		refresh();
	}

	// if client receives a message "GOLD received unclaimed total", assign those values to n, r and p
	if (strncmp(message, "GOLD", 4) == 0) {
		clear();
		int n, p, r;
		sscanf(message, "GOLD %d %d %d", &n, &p, &r);

		// if the client is a player, display game status for a player
		if (isPlayer) { 
			mvprintw(0, 0, "Player %c has %d nuggets (%d nuggets unclaimed).", letter, p, r);
			if (n > 0) {
				mvprintw(0, 50, "GOLD received: %d  ", n);
			}

		// if the client is a spectator, display game status for a spectator
		} else {
			mvprintw(0, 0, "Spectator: %d nuggets unclaimed", r);
		}
		refresh();
	}

	// if client receives a message "DISPLAY string", display the string (map)
	if (strncmp(message, "DISPLAY", 7) == 0) {
		mvprintw(1, 0, "\n");
		const char* map = message + 8;

		fprintf(stderr, "Displaying map...\n");
		for (int i = 0; map[i] != '\0'; i++) {
			fprintf(stderr, "%c", map[i]);
		}

		for (int i = 0; map[i] != '\0'; i++) {
			printw("%c", map[i]);
		}
		mvprintw(NR+2, 0, "\n");
		refresh();
	}

	// if client receives a message "QUIT summary", display the received summary
	if (strncmp(message, "QUIT", 4) == 0) {
		const char* summary = message + 5;
		for (int i = 0; summary[i] != '\0'; i++) {
			printw("%c", summary[i]);
		}
		fprintf(stderr, "Quitting...\n");
		refresh();
		return true;
	}

	// if client receives a message "ERROR explanation", display the explanation in the upper right corner
	if (strncmp(message, "ERROR", 5) == 0) {
		const char* explanation = message + 6;
		move(0, 50);
		for (int i = 0; explanation[i] != '\0'; i++) {
			printw("%c", explanation[i]);
		}
		refresh();
	}

    return false;
}