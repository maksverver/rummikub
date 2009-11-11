#include "rummikub.h"
#include <algorithm>
#include <ctime>
#include <sstream>
#include <utility>
#include <unistd.h>

int get_rand_int(int bound) { return random()%bound; }

static TileList create_random_pool(unsigned seed)
{
    TileList pool;

    srandom(seed);
    pool.reserve(4*13*2);
    for (int color = 0; color < 4; ++color)
    {
        for (int value = 1; value <= 13; ++value)
        {
            for (int dupes = 0; dupes < 2; ++dupes)
            {
                pool.push_back(Tile(color, value));
            }
        }
    }

    std::random_shuffle(pool.begin(), pool.end(), get_rand_int);

    return pool;
}

static GameState create_game_state(unsigned seed)
{
    GameState res;
    TileList pool = create_random_pool(seed);
    TileList::iterator it = pool.begin();

    res.next_player = 0;
    for (int player = 0; player < 4; ++player)
    {
        res.player_tiles[player] = TileList(it, it + 14);
        it += 14;
    }
    res.pool_tiles = TileList(it, pool.end());
    return res;
}

std::ostream &operator<<(std::ostream &os, const GameState &gs)  // DEBUG
{
    os << "Next player: " << gs.next_player + 1 << '\n';
    for (int player = 0; player < 4; ++player)
    {
        os << "Player " << player + 1 << ": " << gs.player_tiles[player] << '\n';
    }
    os << "Table: " << gs.table_tiles << '\n';
    os << "Pool: " << gs.pool_tiles << '\n';
    return os;
}

int main()
{
    std::ostream &xscr = std::cout;

    // Create a new random game
    unsigned seed = time(NULL) ^ (getpid() << 16);
    GameState gs = create_game_state(seed);

    const char *player_name[4] = {
        "Simple 1",
        "Simple 2",
        "Simple 3",
        "Simple 4" };

    const char *player_url[4] = {
        "http://hell.student.utwente.nl/rummikub/simple-player.php",
        "http://hell.student.utwente.nl/rummikub/simple-player.php",
        "http://hell.student.utwente.nl/rummikub/simple-player.php",
        "http://hell.student.utwente.nl/rummikub/simple-player.php" };
    bool player_method[4] = { 1, 1, 1, 1 };

    xscr << "<?xml version='1.0' encoding='UTF-8' ?>"
         << "<rummikub-transcript>\n";

    xscr << " <setup>\n"
         << "  <seed>" << seed << "</seed>\n"
         << "  <players>\n";
    for (int player = 0; player < 4; ++player)
    {
        xscr << "   <player id='" << player + 1 << "'>\n"
             << "    <name>" << player_name[player] << "</name>\n"
             << "    <tiles>" << gs.player_tiles[player] << "</tiles>\n"
             << "   </player>\n";
    }
    xscr << "  </players>\n"
         << "  <pool size='" << gs.pool_tiles.size() << "'>"
         << gs.pool_tiles << "</pool>\n"
         << " </setup>\n\n";

    int turn_no = 0;
    while (!gs.is_game_over())
    {
        ++turn_no;

        const TileList &tiles = gs.player_tiles[gs.next_player];
        TileList played_tiles;
        Table new_table;

        const char *url = player_url[gs.next_player];
        bool method = player_method[gs.next_player];
        std::string response, error;

        xscr << " <turn no='" << turn_no << "' player='"
             << gs.next_player + 1 << "'>\n";

        if (!rpc_move(url, method, 6, gs, response))
        {
            error = "RPC failed";
        }
        else
        {
            if (response.compare(0, 4, "draw") != 0)
            {
                std::istringstream iss(response);
                if (!(iss >> new_table))
                {
                    error = "syntax error in request response";
                }
                else
                {
                    normalize(new_table);
                    if (!gs.move(new_table, &played_tiles))
                    {
                        error = "invalid table configuration";
                    }
                }
            }
        }

        if (played_tiles.empty())
        {
            Tile drawn(0);
            gs.draw(&drawn);
            xscr << "  <drawn>" << drawn << "</drawn>\n";
        }
        else
        {
            xscr << "  <played>" << played_tiles << "</played>\n";
        }

        xscr << "  <table>" << new_table << "</table>\n"
             << "  <tiles value='" << total_value(tiles) << "'>"
             << tiles << "</tiles>\n"
             << "  <pool size='" << gs.pool_tiles.size() << "'/>\n";

        if (!error.empty()) xscr << "  <error>" << error << "</error>\n";

        xscr << " </turn>\n";
    }

    xscr << "\n <scores>\n";
    {
        std::pair<int, int> score_player[4];
        for (int i = 0; i < 4; ++i)
        {
            score_player[i].first  = gs.score(i);
            score_player[i].second = i;
        }
        std::sort(score_player, score_player + 4);
        for (int i = 0; i < 4; ++i)
        {
            int v = score_player[i].first;
            int p = score_player[i].second;
            xscr << "  <score>\n"
                 << "    <player id='" << p + 1 << "'>"
                 << player_name[p] << "</player>\n"
                 << "    <value>" << v << "</value>\n"
                 << "  </score>\n";
        }
    }
    xscr << " </scores>\n"
         << "</rummikub-transcript>\n";
}
