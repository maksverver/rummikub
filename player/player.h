#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

/* Common header files: */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
typedef unsigned char bool;
#define true  1
#define false 0
#endif

/* Game parameters: */
#define V   13        /* number of tile values */
#define C    4        /* number of tile colors */
#define K    2        /* number of duplicates of each tile */

/* Determines the penalty score for a tile with face value v (0 <= v < V) */
#define TILE_VALUE(v)  (1 + (v))

/* A TileColection is an array of counts for each type of tile. */
typedef int TileCollection[V][C];

/* The game state which is presented as input to the player: */
typedef struct GameState
{
    TileCollection tiles;       /* tiles on my rack */
    TileCollection table;       /* tiles on the table */
    int pool_size;              /* number of tiles left in the pool */
    int opponents_tiles[3];     /* number of tiles my opponents have left */
} GameState;

/* A single valid set of tiles, which is either a run or a group: */
typedef struct Set
{
    struct Set *next;
    enum SetType { RUN, GROUP } type;
    union {
        struct Run   { int color, start, length; } run;
        struct Group { int value, color_mask;    } group;
    };
} Set;

/* Utility functions to allocate/free data structure: */
Set *alloc_group(int value, int color_mask, Set *next);
Set *alloc_run(int color, int start, int length, Set *next);
void free_table(Set *set);

/* Functions to compute the value of tables/tile collections: */
int table_value(Set *set);
int collection_value(const int set[V][C]);

/* The core calculation function: calculates the maximum total value of tiles
   that can be placed on the table, and returns the corresponding assignment
   in *table if `table' is not NULL. */
int max_value(const GameState *gs, Set **table);

/* The core player function: determines the next move to make in a given
   gamestate. Returns a new table to play, or NULL to draw/pass. */
Set *play(const GameState *gs);

#endif /* PLAYER_H_INCLUDED */
