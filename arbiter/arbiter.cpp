#include "rummikub.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include <fcntl.h>
#include <unistd.h>

#if 0
static std::ostream &operator<<(std::ostream &os, const GameState &gs)
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
#endif

static const int rpc_timeout = 10;  // seconds

struct Player
{
    std::string name;
    std::string url;
    bool        post;
    int         fd;
};

class FileLock
{
public:
    FileLock(int fd) : fd_(fd) { lock(true); }
    ~FileLock() { lock(false);  }
private:
    void lock(bool lock) {
        if (lockf(fd_, lock ? F_LOCK : F_ULOCK, 0) != 0)
            perror("lockf() failed");
    }
    int fd_;
};

double now()
{
    struct timespec tp = { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec + tp.tv_nsec/1e9;
}

bool load_player(const char *path, Player &player)
{
    std::string line;
    std::fstream ifs(path);
    std::getline(ifs, player.name);
    std::getline(ifs, player.url);
    std::getline(ifs, line);
    if (line == "GET")  player.post = false; else
    if (line == "POST") player.post = true; else return false;
    if (!ifs) return false;
    player.fd = open(path, O_RDWR);
    if (player.fd < 0) return false;
    return true;
}

bool load_tiles(const char *path, TileList &tiles)
{
    std::ifstream ifs(path);
    return (ifs >> tiles) && tiles.size() == 2*13*4;
}

void run_game(std::ostream &xscr, const TileList &tiles, Player (&players)[4])
{
    GameState gs(tiles);

    xscr << "<?xml version='1.0' encoding='UTF-8' ?>"
            "<rummikub-transcript>\n";

    xscr << " <setup>\n"
         << "  <players>\n";
    for (int i = 0; i < 4; ++i)
    {
        const Player &pl = players[i];
        xscr << "   <player id='" << i + 1 << "'>\n"
                "    <name>" << pl.name << "</name>\n"
                "    <url>" << pl.url
                 << "</url>\n"
                "    <method>" << (pl.post ? "POST" : "GET") << "</method>\n"
                "    <tiles>" << gs.player_tiles[i] << "</tiles>\n"
                "   </player>\n";
    }
    xscr << "  </players>\n"
            "  <pool size='" << gs.pool_tiles.size() << "'>" << gs.pool_tiles <<
              "</pool>\n"
            " </setup>\n\n" << std::flush;

    xscr << " <turns>\n";
    int turn_no = 0;
    while (!gs.is_game_over())
    {
        ++turn_no;

        const TileList &tiles = gs.player_tiles[gs.next_player];
        const Player &pl = players[gs.next_player];
        FileLock fl(pl.fd);

        std::string response, error;
        TileList played_tiles;
        Table new_table;

        double delay = now();
        bool ok = rpc_move(pl.url.c_str(), pl.post, rpc_timeout, gs, response);
        delay = now() - delay;

        xscr << "  <turn no='" << turn_no << "' player='"
             << gs.next_player + 1 << "' rpc-delay='" << delay << "s'>\n";

        if (!ok)
        {
            error = "RPC failed";
        }
        else
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

        if (played_tiles.empty())
        {
            Tile drawn(0);
            if (gs.draw(&drawn))
            {
                xscr << "   <drawn>" << drawn << "</drawn>\n"
                     << "   <pool size='" << gs.pool_tiles.size() << "'/>\n";
            }
            else
            if (gs.pass())
            {
                xscr << "   <pass count='" << gs.pass_count << "'/>\n";
            }
        }
        else
        {
            xscr << "   <played>" << played_tiles << "</played>\n"
                    "  <table>" << new_table << "</table>\n";
        }

        xscr << "   <tiles value='" << total_value(tiles) << "'>"
             << tiles << "</tiles>\n";

        if (!error.empty())
        {
            xscr << "   <error>" << error << "</error>\n";
            xscr << "   <response><![CDATA[" << response << "]]></response>\n";
        }

        xscr << "  </turn>\n" << std::flush;
    }
    xscr << " </turns>\n\n"
            " <scores>\n";
    for (int i = 0; i < 4; ++i)
        xscr << "  <score player='" << i + 1 << "'>" << gs.score(i) << "</score>\n";
    xscr << " </scores>\n"
         << "</rummikub-transcript>\n" << std::flush;
}

int main(int argc, const char *argv[])
{
    if (argc != 6)
    {
        std::cout << "usage: arbiter <player1> <player2> <player3> <player4> "
                     "<tiles>" << std::endl;
        return 1;
    }

    Player players[4];
    for (int i = 0; i < 4; ++i)
    {
        if (!load_player(argv[1 + i], players[i]))
        {
            std::cout << "Couldn't read player data from: "
                      << argv[1 + i] << std::endl;
            return 1;
        }
    }

    TileList tiles;
    if (!load_tiles(argv[5], tiles))
    {
        std::cout << "Couldn't load tile data from: "
                  << argv[5] << std::endl;
        return 1;
    }

    run_game(std::cout, tiles, players);
}
