/*
 *
 * gametest.c - test structure for game.c file
 *
 * JL3, CS 50, Fall 2024
 *
 */

#include <stdio.h>
#include "grid.h"

int main(int argc, char const *argv[])
{
    printf("hello\n");
    char *map_path = "../maps/main.txt";
    // int seed = 8;
    // game_t *game = game_new(map_path, seed, );

    // malloc space for map
    char *mapOG = mem_malloc(sizeof(char) * 10000);
    mapOG[0] = '\0'; // added so can use strcat with blank string

    // figure out map size for grid structure
    int height = 0;
    int width = 0;

    // open map and check its readable
    FILE *fp = fopen(map_path, "r");
    if (fp == NULL) { return 1; }

    char *line;
    while ((line = file_readLine(fp)) != NULL)
    {
        strcat(mapOG, line);
        if (height == 0)
        {
            width = strlen(mapOG);
        }
        mem_free(line);
        height++;
    }

    grid_t *grid = grid_new(height, width);

    grid_display(grid, mapOG);

    printf("display check\n");

    char* displayMsg = malloc(10 + strlen(mapOG) + height);
    displayMsg[0] = '\0';

    for(int y = 0; y < height; y++) {
      char* line = mem_malloc(width + 2);
      for (int x = 0; x < width; x++){
        int c = (y * width) + x;
        line[x] = mapOG[c];
      }
      line[width] = '\n';
      strcat(displayMsg, line);
      free(line);
    }

    printf("%s", displayMsg);

    char* mapVisible = mem_malloc(sizeof(char) * 10000);
    memset(mapVisible, ' ', 9999);
    mapVisible[10000] = '\0'; // added so can use strcat with blank string


    // printf("\ngrid_getChar: %c\n", grid_getChar(grid, mapOG, 4, 2));

//  Putting characters in mapOGmapOG
    char* mapCurr = mem_malloc(sizeof(char) * 10000);
    strcpy(mapCurr, mapOG);
    if(!grid_putChar(grid, mapCurr, 4, 2, 'G')) printf("\n grid_putChar didn't work\n");
    if(!grid_putChar(grid, mapCurr, 6, 3, 'T')) printf("\n grid_putChar didn't work\n");

    // grid_display(grid, mapCurr);
    
    //printf("Displaying visible map: \n%s\n", mapVisible);

//  Vis testing
    // printf("\n\nVisibility testing for Mr. J. Beecher\n");

    // grid_addVisiblePoints(grid, mapOG, mapCurr, mapVisible, 13, 2);
    // grid_display(grid, mapVisible);
    // grid_addVisiblePoints(grid, mapOG, mapCurr, mapVisible, 14, 2);
    // grid_display(grid, mapVisible);
    // grid_addVisiblePoints(grid, mapOG, mapCurr, mapVisible, 15, 2);
    // grid_display(grid, mapVisible);

    // grid_addVisiblePoints(grid, mapOG, mapCurr, mapVisible, 16, 7);
    // grid_display(grid, mapVisible);
    grid_delete(grid);
    mem_free(mapVisible);
    mem_free(mapCurr);
    mem_free(mapOG);
    fclose(fp);

    return 0;
}
