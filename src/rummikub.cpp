#include "rummikub.h"
#include <algorithm>
#include <set>

// Returns the sum of the values of the given tiles
int total_value(const TileList &tiles)
{
    int res = 0;
    for (TileList::const_iterator it = tiles.begin(); it != tiles.end(); ++it)
        res += it->value();
    return res;
}

// Verifies that all tiles have the same value, but all different colors.
// Assumes `tiles' is not empty.
static bool is_group(const TileList &tiles)
{
    int value = tiles[0].value();
    int used_colors = 0;
    for (TileList::const_iterator it = tiles.begin(); it != tiles.end(); ++it)
    {
        if (it->value() != value) return false;
        int color_mask = 1 << it->color();
        if (used_colors & color_mask) return false;
        used_colors |= color_mask;
    }
    return true;
}

// Verifies that all tiles have the same color and consecutive values.
// Assumes `tiles' is normalized and not empty.
static bool is_run(const TileList &tiles)
{
    int color = tiles[0].color();
    int next_value = tiles[0].value();
    for (TileList::const_iterator it = tiles.begin(); it != tiles.end(); ++it)
    {
        if (it->color() != color || it->value() != next_value) return false;
        ++next_value;
    }
    return true;
}

// Verifies that `tiles' is a valid set of tiles to be placed on the table.
// Assumes `tiles' is normalized.
bool is_valid(const TileList &tiles)
{
    return tiles.size() >= 3 && (is_group(tiles) || is_run(tiles));
}

// Verifies that `table' is a valid table configuration; i.e. all sets on the
// table are valid. Assumes `table' is partially normalized.
bool is_valid(const Table &table)
{
    for (Table::const_iterator it = table.begin(); it != table.end(); ++it)
        if (!is_valid(*it)) return false;
    return true;
}

void normalize(TileList &tiles)
{
    std::sort(tiles.begin(), tiles.end());
}

void normalize(Table &table)
{
    for (Table::iterator it = table.begin(); it != table.end(); ++it)
    {
        normalize(*it);
    }
}

// Constructs a new game state for the given random seed
GameState::GameState(const TileList &pool)
{
    TileList::const_iterator it = pool.begin();

    next_player = 0;
    for (int player = 0; player < 4; ++player)
    {
        player_tiles[player] = TileList(it, it + 14);
        it += 14;
    }
    pool_tiles = TileList(it, pool.end());
}

// Returns whether the game is over
bool GameState::is_game_over()
{
    if (pool_tiles.empty()) return true;
    for (int player = 0; player < 4; ++player)
        if (player_tiles[player].empty()) return true;
    return false;
}

bool GameState::draw(Tile *drawn_out)
{
    if (pool_tiles.empty()) return false;
    Tile drawn = *pool_tiles.begin();
    pool_tiles.erase(pool_tiles.begin());
    player_tiles[next_player].push_back(drawn);
    next_player = (next_player + 1)%4;
    if (drawn_out) *drawn_out = drawn;
    return true;
}

static TileList table_to_sorted_list(const Table &table)
{
    TileList res;
    for (Table::const_iterator it = table.begin(); it != table.end(); ++it)
        res.insert(res.end(), it->begin(), it->end());
    std::sort(res.begin(), res.end());
    return res;
}

bool GameState::move(const Table &new_table, TileList *played_out)
{
    if (!is_valid(new_table)) return false;

    TileList old_list = table_to_sorted_list(table_tiles),
             new_list = table_to_sorted_list(new_table);

    // Verify that all tiles in the old table configuration were used:
    if (!std::includes(new_list.begin(), new_list.end(),
                       old_list.begin(), old_list.end())) return false;

    // Determine which new tiles the player played:
    TileList played;
    std::set_difference(new_list.begin(), new_list.end(),
                        old_list.begin(), old_list.end(),
                        std::back_insert_iterator<TileList>(played));

    // Verify that at least one new tile was played:
    if (played.empty()) return false;

    // Verify that only player's tiles were played:
    TileList new_tiles = player_tiles[next_player];
    for (TileList::const_iterator i = played.begin(); i != played.end(); ++i)
    {
        TileList::iterator j = find(new_tiles.begin(), new_tiles.end(), *i);
        if (j == new_tiles.end()) return false;
        new_tiles.erase(j);
    }

    // All OK!
    if (played_out) played_out->swap(played);
    table_tiles = new_table;
    player_tiles[next_player] = new_tiles;
    next_player = (next_player + 1)%4;
    return true;
}

int GameState::score(int player)
{
    return total_value(player_tiles[player]);
}
