#include "player.h"
#include <stdio.h>

static const char *colors = "RGBK";

static void parse_tiles(char *str, TileCollection set)
{
    char *tok, *ptr;

    memset(set, 0, sizeof(TileCollection));
    for (tok = strtok_r(str, ".-", &ptr); tok; tok = strtok_r(NULL, ".-", &ptr))
    {
        char *p = strchr(colors, tok[0]);
        int v = atoi(tok + 1) - 1, c = p ? p - colors : -1;
        if (v >= 0 && v < V && c >= 0 && c < C && set[v][c] < K) ++set[v][c];
    }
}

static void parse_query(char *query, GameState *gs)
{
    char *key, *val, *ptr;

    memset(gs, 0, sizeof(GameState));
    for (key = strtok_r(query, "=", &ptr); key; key = strtok_r(NULL, "=", &ptr))
    {
        if ((val = strtok_r(NULL, "&;", &ptr)) != NULL)
        {
            if (strcmp(key, "yourTiles") == 0)
                parse_tiles(val, gs->tiles);
            if (strcmp(key, "table") == 0)
                parse_tiles(val, gs->table);
            if (strcmp(key, "poolTiles") == 0)
                gs->pool_size = atoi(val);
            if (strcmp(key, "opponentsTiles") == 0)
                sscanf(val, "%d.%d.%d", &gs->opponents_tiles[0],
                    &gs->opponents_tiles[1], &gs->opponents_tiles[2]);
        }
    }
}

static bool parse_cgi_args(const char *method, GameState *gs)
{
    char buf[1024];

    if (strcmp(method, "GET") == 0)
    {
        const char *qs = getenv("QUERY_STRING");
        if (qs == NULL || qs[0] == '\0')
        {
            printf("No/empty query string received.\n");
            return false;
        }
        strncpy(buf, qs, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
    }
    else
    if (strcmp(method, "POST") == 0)
    {
        if (!fgets(buf, sizeof(buf), stdin) || buf[0] == '\0')
        {
            printf("No/empty request body received.\n");
            return false;
        }
    }
    else
    {
        printf("Invalid request method received (%s).\n", method);
        return false;
    }

    parse_query(buf, gs);
    return true;
}

static void print_tile(int value, int color, FILE *fp)
{
    fprintf(fp, "%c%d", colors[color], value + 1);
}

static void print_set(Set *set, FILE *fp)
{
    switch (set->type)
    {
    case RUN:
        {
            int i;
            for (i = 0; i < set->run.length; ++i)
            {
                if (i > 0) fputc('.', fp);
                print_tile(set->run.start + i, set->run.color, fp);
            }
        } break;

    case GROUP:
        {
            bool first = true;
            int c;
            for (c = 0; c < C; ++c)
            {
                if (set->group.color_mask & (1 << c))
                {
                    if (!first) fputc('.', fp);
                    print_tile(set->group.value, c, fp);
                    first = false;
                }
            }
        } break;

    default: assert(0);
    }
}

static void print_table(Set *set, FILE *fp)
{
    for ( ; set != NULL; set = set->next)
    {
        print_set(set, fp);
        if (set->next) fputc('-', fp);
    }
    fputc('\n', stdout);
}

int main(int argc, char *argv[])
{
    const char *method;
    GameState gs;
    Set *new_table;

    if ((method = getenv("REQUEST_METHOD")) != NULL)
    {
        /* Use CGI interface: */
        printf("Content-Type: text/plain\n\n");
        if (!parse_cgi_args(method, &gs)) return 1;
    }
    else
    {
        /* Use command-line interface: */
        if (argc != 2)
        {
            fprintf(stderr, "Usage: player <query>\n");
            return 1;
        }
        parse_query(argv[1], &gs);
    }

    /* Call play() to determine what to do: */
    if ((new_table = play(&gs)) != NULL)
    {
        /* Playing */
        print_table(new_table, stdout);
        free_table(new_table);
    }
    else
    {
        /* Drawing */
        fputs("draw\n", stdout);
    }

    return 0;
}
