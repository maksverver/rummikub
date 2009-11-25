/* In this file the high-level strategy of the player is implemented. */

#include "player.h"

#ifndef GREEDY
/* Removes a set from a tile collection, and returns its total value. */
static int reserve_set(TileCollection tiles)
{
    int v, c;

    for (v = 0; v < V; ++v)
    {
        /* Search for a group: */
        int cnt = 0;
        for (c = 0; c < C; ++c) if (tiles[v][c] > 0) ++cnt;
        if (cnt >= 3)
        {
            /* Found a group! */
            for (c = 0; c < C; ++c) if (tiles[v][c] > 0) --tiles[v][c];
            return cnt*TILE_VALUE(v);
        }

        /* Search for a run: */
        for (c = 0; c < C; ++c)
        {
            int len = 0;
            while (v + len < V && tiles[v + len][c] > 0) ++len;
            if (len >= 3)
            {
                /* Found a run! */
                int res = 0, w;
                for (w = v; w < len + v; ++w)
                {
                    res += TILE_VALUE(w);
                    --tiles[w][c];
                }
                return res;
            }
        }
    }

    return 0;
}
#endif /* def GREEDY */

/* This function implements the high-level strategy of the player. It takes a
   valid game state and either returns a new table configuration, or NULL if no
   tiles can be played.

   The strategy is to minimize the total value of tiles left on rack, while
   trying to hold back valid sets of tiles that can be played later without
   requiring any specific combination of tiles on the table. (The tiles in these
   sets are not counted against the value of tiles left.)

   When this file is compiled with GREEDY defined, a simpler strategy is used,
   which never holds back any tiles. This results in a slightly faster game, but
   may cause the player to draw more frequently.
*/
Set *play(const GameState *gs)
{
    /* Determine the total value of the tiles on table and on rack: */
    int table_value = collection_value(gs->table);
    int total_value = table_value + collection_value(gs->tiles);

    /* Find the maximum value of tiles that can be placed on table: */
    Set *best_result;
    int best_value = max_value(gs, &best_result);

    assert(best_value >= table_value && best_value <= total_value);

    /* Check if we can play any new tiles (if not, we are forced to draw) */
    if (best_value == table_value)
    {
        free_table(best_result);
        return NULL;
    }

#ifndef GREEDY
    /* Try to hold back valid sets only if we cannot empty the rack: */
    if (best_value < total_value)
    {
        GameState new_gs = *gs;
        Set *new_result;
        int reserved = 0, value;

        while ((value = reserve_set(new_gs.tiles)) > 0)
        {
            reserved += value;
            value = max_value(&new_gs, &new_result);
            assert(reserved + value <= best_value);
            if (value == table_value || reserved + value < best_value) break;
            free_table(best_result);
            best_result = new_result;
        }
    }
#endif /* ndef GREEDY */

    return best_result;
}
