#include "player.h"

/* Convenience macros: */
#define FOR(i, a, b) for(i = a; i < b; ++i)
#define REP(i, n) FOR(i, 0, n)

static const short inf = 32767;  /* stored in memo, so must be short! */

typedef struct CalcState
{
    int     lens[C][K];         /* lengths of current runs (0..3) (per color) */
    int     runs[C];            /* #tiles assigned to runs   (per color) */
    int     grps[C];            /* #tiles assigned to groups (per color) */
} CalcState;

/* Computes the memo key for the given (partial) computation state: */
static size_t get_memo_key(const int lens[C][K], int cur_v)
{
    size_t res = 0;
    int c, k;

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

/* Greedily extends the first `cnt' runs and terminates the remaining runs.
   Assumes input lengths are normalized (so that shorter runs appear first, and
   empty runs appear at the end) and produces normalized output as well. */
static void extend_runs(int lengths[K], int cnt)
{
    int i, j;

    assert(cnt <= K);

    REP(i, cnt) if (lengths[i] < 3) ++lengths[i];
    FOR(i, cnt, K) {
        assert(lengths[i] == 0 || lengths[i] == 3);
        lengths[i] = 0;
    }

    /* Normalize the result by bubble sorting: */
    REP(i, K) FOR(j, i + 1, K) if ((lengths[i] + 3)%4 > (lengths[j] + 3)%4) {
        int tmp = lengths[i];
        lengths[i] = lengths[j];
        lengths[j] = tmp;
    }
}

/* Determines if the given tiles (where tile[c] is the number of tiles of color
   c) can be grouped in a valid way. By the pigeon hole principle, if we have
   `x' tiles of one color, we must make at least `x' groups of 3 tiles.
   (Interestingly, this property is sufficent as well as neccessary.) */
static bool can_group(const int tiles[C])
{
    int c, total = 0;
    REP(c, C) total += tiles[c];
    REP(c, C) if (3*tiles[c] > total) return false;
    return true;
}

static int calc( const GameState *gs, const CalcState *cs, short memo[],
                 int cur_v, int cur_c )
{
    CalcState new_cs;
    int res = -inf, c;
    short *mem = (cur_c == 0) ? &memo[get_memo_key(cs->lens, cur_v)] : NULL;

    if (cur_c == C)  /* at end of current value */
    {
        if (!can_group(cs->grps)) return -inf;

        /* Discard grouped tiles and move on to next value: */
        new_cs = *cs;
        REP(c, C) new_cs.grps[c] = 0;
        return calc(gs, &new_cs, memo, cur_v + 1, 0);
    }

    if (mem != NULL && *mem != -1) return *mem;

    if (cur_v == V)
    {
        /* Check for unfinished runs: */
        res = 0;
        REP(c, C) if (get_min_extended_runs(cs->lens[c]) > 0) res = -inf;
    }
    else  /* cur_v < V */
    {
        int min_use, max_use, min_run, use, in_run, val;

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
                new_cs = *cs;
                extend_runs(new_cs.lens[cur_c], in_run);
                assert(new_cs.grps[cur_c] == 0);
                new_cs.grps[cur_c] = use - in_run;

                /* Recursively calculate value: */
                val = TILE_VALUE(cur_v)*use;
                val += calc(gs, &new_cs, memo, cur_v, cur_c + 1);
                if (val > res) res = val;
            }
        }
    }

    if (res < 0) res = -inf;
    if (mem != NULL) *mem = res;
    return res;
}

/* Figures out the optimal assignment of tiles of a fixed value (`cur_v') and
   return true, leaving the assignment in `cs'. When called with cur_c == 0 and
   a valid memo and value, this function should always return true. */
static bool reconstruct_for_v( const GameState *gs, CalcState *cs,
                               short memo[], int cur_v, int cur_c, int *value )
{
    if (cur_c == C)
    {
        if (!can_group(cs->grps)) return false;

        /* Valid configuration; check if this is optimal: */
        {
            short v = memo[get_memo_key((const int(*)[K])cs->lens, cur_v + 1)];
            assert(v != -1);      /* memo entry must be set! */
            assert(v <= *value);  /* value is expected to be optimal */
            return v == *value;
        }
    }
    else  /* cur_c < C */
    {
        int min_use, max_use, min_run, use, in_run;
        int old_lens[K], k;

        min_use = gs->table[cur_v][cur_c];
        max_use = gs->table[cur_v][cur_c] + gs->tiles[cur_v][cur_c];
        min_run = get_min_extended_runs(cs->lens[cur_c]);

        /* Backup run lengths: */
        REP(k, K) old_lens[k] = cs->lens[cur_c][k];

        /* Determine how many tiles of the current value/color we will use */
        *value -= TILE_VALUE(cur_v)*min_use;
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

            *value -= TILE_VALUE(cur_v);
        }
        *value += TILE_VALUE(cur_v)*use;
        return false;
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
        if ( (runs[i] ? runs[i]->run.length : inf) >
             (runs[j] ? runs[j]->run.length : inf) )
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

static Set *reconstruct(const GameState *gs, short memo[], int value)
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

int max_value(const GameState *gs, Set **table)
{
    size_t memo_size = (V + 1)*(1<<(2*C*K));
    short *memo;
    CalcState cs;
    int value;

    /* Initialize memo: */
    memo = malloc(sizeof(*memo)*memo_size);
    assert(memo != NULL);
    memset(memo, -1, sizeof(*memo)*memo_size);

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
