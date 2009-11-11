#ifndef RUMMIKUB_H_INCLUDED
#define RUMMIKUB_H_INCLUDED

#include <iostream>
#include <vector>

struct Tile
{
    Tile(int color, int value) : id(13*color + value - 1) { }
    Tile(int id) : id(id) { }
    int value() const { return id%13 + 1; }
    int color() const { return id/13; }
    operator int() const { return id; }
private:
    int id;
};

typedef std::vector<Tile> TileList;
typedef std::vector<TileList> Table;

struct GameState
{
    int         next_player;
    TileList    player_tiles[4];
    Table       table_tiles;
    TileList    pool_tiles;

    bool is_game_over();
    bool draw(Tile *drawn = 0);
    bool move(const Table &new_table, TileList *played = 0);
    int score(int player);
};

int total_value(const TileList &tiles);
void normalize(TileList &tiles);
void normalize(Table &table);
bool is_valid(const TileList &tiles);
bool is_valid(const Table &table);

// I/O operations defined in rummikub-io.cpp
std::ostream &operator<<(std::ostream &os, const Tile &t);
std::ostream &operator<<(std::ostream &os, const TileList &vt);
std::ostream &operator<<(std::ostream &os, const Table &vvt);
std::istream &operator>>(std::istream &is, Tile &t);
std::istream &operator>>(std::istream &is, TileList  &vt);
std::istream &operator>>(std::istream &is, Table &vvt);

// RPC operations defined in rummikub-rpc.cpp
bool rpc_move(const char *url, bool post, int timeout,
              const GameState &game_state, std::string &response);

#endif /* ndef RUMMIKUB_H_INCLUDED */
