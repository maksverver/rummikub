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

Set *play(const GameState *gs)
{
    int table_value = collection_value(gs->table);
    int total_value = table_value + collection_value(gs->tiles);
    Set *best_result;
    int best_value = max_value(gs, &best_result);

    assert(best_value >= table_value && best_value <= total_value);
    if (best_value == table_value)
    {
        /* Can't play anything now -- just draw */
        free_table(best_result);
        return NULL;
    }

#ifndef GREEDY
    if (best_value < total_value)  /* table_value < value < total_value */
    {
        /* Try to hold back valid sets (to play instead of drawing later): */
        GameState new_gs = *gs;
        int reserved = 0, value;

        while ((value = reserve_set(new_gs.tiles)) > 0)
        {
            Set *new_result;

            reserved += value;
            value = max_value(&new_gs, &new_result);
            assert(reserved + value <= best_value);

            /* Check if we can still play an equally good meld: */
            if (value == table_value || reserved + value < best_value) break;

            free_table(best_result);
            best_result = new_result;
        }
    }
#endif /* ndef GREEDY */

    return best_result;
}
