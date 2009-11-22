#include "player.h"

/* Removes a set from a tile collection, and returns its total value. */
static int reserve_set(TileCollection tiles)
{
    int v, c;

    /* Search for a group: */
    for (v = 0; v < V; ++v)
    {
        int cnt = 0;
        for (c = 0; c < C; ++c) if (tiles[v][c] > 0) ++cnt;
        if (cnt >= 3)
        {
            /* Found a group! */
            for (c = 0; c < C; ++c) if (tiles[v][c] > 0) --tiles[v][c];
            return cnt*(v + 1);
        }
    }

    /* Search for a run: */
    for (c = 0; c < C; ++c)
    {
        for (v = 0; v < V; ++v)
        {
            int len = 0;
            while (v + len < V && tiles[v + len][c] > 0) ++len;
            if (len >= 3)
            {
                /* Found a run! */
                int w;
                for (w = v; w < v + len; ++w) --tiles[w][c];
                return len*(2*v + len + 1)/2;
            }
        }
    }

    return 0;
}

/* "This does everything, and consequently is very simple." (A.P.) */
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

    if (best_value < total_value)  /* table_value < value < total_value */
    {
        /* Try to hold back a combination: */
        GameState new_gs = *gs;
        int reserved_value = reserve_set(new_gs.tiles);
        if (reserved_value > 0)
        {
            Set *new_result;
            int new_value;

            /* Check if this is just as good as playing all we can: */
            new_value = max_value(&new_gs, &new_result);
            assert(new_value + reserved_value <= best_value);
            if ( new_value > table_value &&
                 new_value + reserved_value == best_value )
            {
                free_table(best_result);
                best_result = new_result;
            }
        }
    }

    return best_result;
}
