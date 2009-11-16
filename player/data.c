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

static int bitcount(int i)
{
    int res = 0;
    for ( ; i; i &= i - 1) ++res;
    return res;
}

int table_value(Set *set)
{
    int res = 0;
    for ( ; set != NULL; set = set->next)
    {
        switch (set->type)
        {
        case RUN:
            res += set->run.length*(2*set->run.start + set->run.length + 1)/2;
            break;
        case GROUP:
            res += bitcount(set->group.color_mask)*(set->group.value + 1);
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
            res += (v + 1)*set[v][c];
    return res;
}
