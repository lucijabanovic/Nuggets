/*
 *
 * gametest.c - test structure for game.c file
 *
 * JL3, CS 50, Fall 2024
 *
 */

#include "game.h"
// #include <stdbool.h>
#include <stdio.h>
#include "grid.h"

int main(int argc, char const *argv[])
{
    game_t *game = game_new("./maps/small.txt");

    // addr_t addy;
    

    // game_addPlayer(game, "Billiam", addy);

    game_delete(game);
    return 0;
}
