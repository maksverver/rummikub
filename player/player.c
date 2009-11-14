#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Game parameters: */
#define V   13        /* number of tile values */
#define C    4        /* number of tile colors */
#define K    2        /* number of duplicates of each tile */

/* Convenience macros: */
#define FOR(i, a, b) for(i = a; i < b; ++i)
#define REP(i, n) FOR(i, 0, n)

typedef int TileSet[V][C];

typedef struct CalcState
{
    int     lens[C][K];         /* lengths of current runs (0..3) (per color) */
    int     runs[C];            /* #tiles assigned to runs   (per color) */
    int     grps[C];            /* #tiles assigned to groups (per color) */
} CalcState;

typedef struct GameState
{
    TileSet tiles;                  /* tiles on my rack */
    TileSet table;                  /* tiles on the table */
    int     pool_size;              /* number of tiles left in the pool */
    int     opponents_tiles[3];     /* number of tiles my opponents have left */
} GameState;


enum SetType { RUN, GROUP };

typedef struct Run      { int color, start, length; } Run;
typedef struct Group    { int value, color_mask; } Group;

typedef struct Set
{
    struct Set *next;
    enum SetType type;
    union {
        Run   run;
        Group group;
    };
} Set;

static const char *colors = "RGBK";
static const int inf = 999999999;

static int total_value(TileSet set)
{
    int res = 0, v, c;
    REP(v, V) REP(c, C) res += (v + 1)*set[v][c];
    return res;
}

static int get_memo_key(const int lens[C][K], int cur_v)
{
    int res = 0, c, k;

    REP(c, C) REP(k, K) res = 4*res + lens[c][k];
    res = V*res + cur_v;

    return res;
}

/* Calculates how many of the given runs must still be extended (i.e. have
   length greater than zero but less than three) */
static int get_min_extended_runs(const int lengths[K])
{
    int k, res = 0;
    REP(k, K) if (lengths[k] > 0 && lengths[k] < 3) ++res;
    return res;
}

/* Greedily extends `cnt' runs, first extending runs of length 1 or 2, then
   runs of length 3, and finally of length 0. */
static void extend_runs(int lengths[K], int cnt)
{
    int i, j;

    /* Bubble sort runs so lengths are ordered as (1,2,3,0) */
    /* CHECKME: is this necessary? */
    REP(i, K) FOR(j, i + 1, K) if ((lengths[i] + 3)%4 > (lengths[j] + 3)%4) {
        int tmp = lengths[i];
        lengths[i] = lengths[j];
        lengths[j] = tmp;
    }

    assert(cnt <= K && (cnt == K || (lengths[cnt] == 0 || lengths[cnt] == 3)));

    REP(i, cnt) lengths[i] += (lengths[i] < 3) ? 1 : 0;
    FOR(i, cnt, K) lengths[i] = 0;
}

static bool can_group(const int tiles[C])
{
    int c, total = 0;

    assert(C == 4);  /* this function doesn't generalize! */

    /* Can't make groups with exactly 1, 2 or 5 tiles. */
    REP(c, C) total += tiles[c];
    if (total == 1 || total == 2 || total == 5) return false;

    /* By the pigeon hole principle, if we have `x' tiles of one color, we must
       make at least `x' groups of 3 tiles: */
    REP(c, C) if (3*tiles[c] > total) return false;

    return true;    /* CHECKME: is this always correct? */
}

static int calc( const GameState *gs, const CalcState *cs, int memo[],
                 int cur_v, int cur_c )
{
    if (cur_c == C)  /* at end of current value */
    {
        CalcState new_cs;
        int c;

        if (!can_group(cs->grps)) return -inf;

        /* Discard grouped tiles and move on to next value: */
        new_cs = *cs;
        REP(c, C) new_cs.grps[c] = 0;
        return calc(gs, &new_cs, memo, cur_v + 1, 0);
    }
    else
    if (cur_v == V)  /* at end of tiles */
    {
        int c;

        /* Check for unfinished runs: */
        REP(c, C) if (get_min_extended_runs(cs->lens[c]) > 0) return -inf;

        return 0;
    }
    else
    {
        int *mem = (cur_c == 0) ? &memo[get_memo_key(cs->lens, cur_v)] : NULL;
        int res, min_use, max_use, min_run, use, in_run, val;

        if (mem != NULL && *mem != -1) return *mem;

        res = -inf;
        min_use = gs->table[cur_v][cur_c];
        max_use = gs->table[cur_v][cur_c] + gs->tiles[cur_v][cur_c];
        min_run = get_min_extended_runs(cs->lens[cur_c]);

        /* Determine how many tiles of the current value/color we will use */
        for (use = min_use; use <= max_use; ++use)
        {
            /* Determine how many used tiles will be placed in runs */
            for (in_run = min_run; in_run <= use; ++in_run)
            {
                /* Calculate updated state: */
                CalcState new_cs = *cs;
                extend_runs(new_cs.lens[cur_c], in_run);
                assert(new_cs.grps[cur_c] == 0);
                new_cs.grps[cur_c] = use - in_run;

                /* Recursively calculate value: */
                val = (cur_v + 1)*use;
                val += calc(gs, &new_cs, memo, cur_v, cur_c + 1);
                if (val > res) res = val;
            }
        }

        if (res < 0) res = -inf;
        if (mem != NULL) *mem = res;
        return res;
    }
}

/* Figures out the optimal assignment of tiles of a fixed value (`cur_v') and
   return true, leaving the assignment in `cs'. When called with cur_c == 0 and
   a valid memo and value, this function should always return true. */
static bool reconstruct_for_v( const GameState *gs, CalcState *cs,
                               int memo[], int cur_v, int cur_c, int *value )
{
    if (cur_c == C)
    {
        if (!can_group(cs->grps)) return false;

        /* Valid configuration; check if this is optimal: */
        {
            int mem = (cur_v + 1 < V) ?
                memo[get_memo_key((const int(*)[K])cs->lens, cur_v + 1) ] : 0;
            assert(mem != -1);      /* memo entry must be set! */
            assert(mem <= *value);  /* value is expected to be optimal */
            return mem == *value;
        }
    }
    else
    {
        int min_use, max_use, min_run, use, in_run;
        int old_lens[K], k;

        min_use = gs->table[cur_v][cur_c];
        max_use = gs->table[cur_v][cur_c] + gs->tiles[cur_v][cur_c];
        min_run = get_min_extended_runs(cs->lens[cur_c]);

        /* Backup run lengths: */
        REP(k, K) old_lens[k] = cs->lens[cur_c][k];

        /* Determine how many tiles of the current value/color we will use */
        *value -= (cur_v + 1)*min_use;
        for (use = min_use; use <= max_use; ++use)
        {
            /* Determine how many used tiles will be placed in runs */
            for (in_run = min_run; in_run <= use; ++in_run)
            {
                extend_runs(cs->lens[cur_c], in_run);
                cs->runs[cur_c] = in_run;
                cs->grps[cur_c] = use - in_run;

                if (reconstruct_for_v(gs, cs, memo, cur_v, cur_c + 1, value))
                    return true;

                /* Restore value and run lengths: */
                REP(k, K) cs->lens[cur_c][k] = old_lens[k];
            }

            *value -= cur_v + 1;
        }
        *value += (cur_v + 1)*use;
        return false;
    }
}

static Set *alloc_group(int value, int color_mask, Set *next)
{
    Set *res = malloc(sizeof(Set));
    assert(res != NULL);
    res->next = next;
    res->type = GROUP;
    res->group.value = value;
    res->group.color_mask = color_mask;
    return res;
}

static Set *alloc_run(int color, int start, int length, Set *next)
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
            REP(c, C) if (set->group.color_mask & (1 << c)) {
                if (!first) fputc('.', fp);
                print_tile(set->group.value, c, fp);
                first = false;
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
}

static int bitcount(int i)
{
    int res = 0;
    for ( ; i; i &= i - 1) ++res;
    return res;
}

static int set_value(Set *set)
{
    switch (set->type)
    {
    case RUN:
        return set->run.length*(2*set->run.start + set->run.length + 1)/2;
    case GROUP:
        return bitcount(set->group.color_mask)*(set->group.value + 1);
    }
    assert(0);
    return -1;
}

static int table_value(Set *set)
{
    int res = 0;
    for ( ; set != NULL; set = set->next) res += set_value(set);
    return res;
}

static void free_table(Set *set)
{
    while (set != NULL)
    {
        Set *next = set->next;
        free(set);
        set = next;
    }
}

static Set *make_groups(int value, int grps[C], Set *list)
{
    assert(C == 4);  /* this function doesn't generalize! */

    for (;;)
    {
        int i, j, cnt, take, mask, idx[C];

        /* Figure out the size of the group to make: */
        cnt = 0;
        REP(i, C) cnt += grps[i];
        take = (cnt%3 == 0) ? 3 : 4;
        if (cnt == 0) break;

        /* Bubble-sort indices by tile counts, highest first: */
        REP(i, C) idx[i] = i;
        REP(i, C) FOR(j, i + 1, C) if (grps[idx[i]] < grps[idx[j]]) {
            int tmp = idx[i];
            idx[i] = idx[j];
            idx[j] = tmp;
        }

        /* Take tiles from the first `take' biggest groups: */
        mask = 0;
        REP(i, take) {
            mask |= 1 << idx[i];
            --grps[idx[i]];
        }

        list = alloc_group(value, mask, list);
    }
    return list;
}

static Set *make_runs(int value, int color, Set *runs[K], int cnt, Set *list)
{
    int i, j;

    /* Bubble-sort existing runs by length (increasing): */
    REP(i, K) FOR(j, i + 1, K) {
        int li = runs[i] != NULL ? runs[i]->run.length : inf;
        int lj = runs[j] != NULL ? runs[j]->run.length : inf;
        if (li > lj)
        {
            Set *tmp = runs[i];
            runs[i] = runs[j];
            runs[j] = tmp;
        }
    }

    assert(cnt <= K);

    /* Extend the first `cnt' runs: */
    REP(i, cnt) {
        if (runs[i] == NULL) runs[i] = list = alloc_run(color, value, 0, list);
        ++runs[i]->run.length;
    }

    /* Terminate the other runs: */
    FOR(i, cnt, K) runs[i] = NULL;

    return list;
}

static Set *reconstruct(const GameState *gs, int memo[], int value)
{
    CalcState cs;
    Set *list = NULL;
    Set *runs[C][K];
    int v, c;

    memset(&cs, 0, sizeof(cs));
    memset(&runs, 0, sizeof(runs));

    REP(v, V) {
        bool ok = reconstruct_for_v(gs, &cs, memo, v, 0, &value);
        assert(ok);
        list = make_groups(v, cs.grps, list);
        REP(c, C) list = make_runs(v, c, runs[c], cs.runs[c], list);
    }
    return list;
}

static int max_value(const GameState *gs, Set **table)
{
    static int *memo;
    CalcState cs;
    size_t memo_size = V*(1<<(2*C*K));
    int value;

    /* Initialize memo: */
    memo = malloc(sizeof(int)*memo_size);
    assert(memo != NULL);
    memset(memo, -1, sizeof(int)*memo_size);

    /* Fill memo recursively: */
    memset(&cs, 0, sizeof(cs));
    value = calc(gs, &cs, memo, 0, 0);

    if (table != NULL)
    {
        /* Reconstruct solution: */
        memset(&cs, 0, sizeof(cs));
        *table = reconstruct(gs, memo, value);
        assert(table_value(*table) == value);
    }

    free(memo);

    return value;
}

/*
    CGI interface follows:
*/

static void parse_tiles(char *str, TileSet set)
{
    char *tok, *ptr;

    memset(set, 0, sizeof(TileSet));
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
            fprintf(stderr, "No/empty query string received.\n");
            return false;
        }
        strncpy(buf, qs, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\n';
    }
    else
    if (strcmp(method, "POST") == 0)
    {
        if (!fgets(buf, sizeof(buf), stdin) || buf[0] == '\0')
        {
            fprintf(stderr, "No/empty request body received.\n");
            return false;
        }
    }
    else
    {
        fprintf(stderr, "Invalid request method received (%s).\n", method);
        return false;
    }

    parse_query(buf, gs);
    return true;
}

int main(int argc, char *argv[])
{
    const char *method;
    GameState gs;
    Set *new_table;
    int new_value;

    if ((method = getenv("REQUEST_METHOD")) != NULL)
    {
        /* Use CGI interface: */
        if (!parse_cgi_args(method, &gs)) return 1;
        printf("Content-Type: text/plain\n\n");
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
    new_value = max_value(&gs, &new_table);
    if (new_value > total_value(gs.table))
        print_table(new_table, stdout);
    else
        fputs("draw", stdout);
    putc('\n', stdout);
    free_table(new_table);
    return 0;
}
