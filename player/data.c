#include "player.h"

Set *alloc_group(int value, int color_mask, Set *next)
{
    Set *res = malloc(sizeof(Set));
    assert(res != NULL);
    res->next = next;
    res->type = GROUP;
    res->group.value = value;
    res->group.color_mask = color_mask;
    return res;
}

Set *alloc_run(int color, int start, int length, Set *next)
{
    Set *res = malloc(sizeof(Set));
    assert(res != NULL);
    res->next = next;
    res->type = RUN;
    res->run.color  = color;
    res->run.start  = start;
    res->run.length = length;
    return res;
}

void free_table(Set *set)
{
    while (set != NULL)
    {
        Set *next = set->next;
        free(set);
        set = next;
    }
}

int table_value(Set *set)
{
    int res = 0, i;
    for ( ; set != NULL; set = set->next)
    {
        switch (set->type)
        {
        case RUN:
            for (i = 0; i < set->run.length; ++i)
                res += TILE_VALUE(set->run.start + i);
            break;
        case GROUP:
            for (i = set->group.color_mask; i; i &= i - 1)
                res += TILE_VALUE(set->group.value);
            break;
        }
    }
    return res;
}

int collection_value(const int set[V][C])
{
    int res = 0, v, c;
    for (v = 0; v < V; ++v)
        for (c = 0; c < C; ++c)
            res += TILE_VALUE(v)*set[v][c];
    return res;
}
