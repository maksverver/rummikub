#include "player.h"

/* "This does everything, and consequently is very simple." (A.P.) */

Set *play(const GameState *gs)
{
    Set *new_table = NULL;
    int new_value = max_value(gs, &new_table);

    if (new_value == collection_value(gs->table))
    {
        free_table(new_table);
        new_table = NULL;
    }

    return new_table;
}
