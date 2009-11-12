#include "rummikub.h"
#include <cstring>

static const char *color_letters = "RGBK";

std::ostream &operator<<(std::ostream &os, const Tile &t)
{
    return os << color_letters[t.color()] << t.value();
}

std::ostream &operator<<(std::ostream &os, const TileList &vt)
{
    for (TileList::const_iterator it = vt.begin(); it != vt.end(); ++it)
    {
        if (it != vt.begin()) os << '.';
        if (!(os << *it)) break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Table &vvt)
{
    for (Table::const_iterator it = vvt.begin(); it != vvt.end(); ++it)
    {
        if (it != vvt.begin()) os << '-';
        if (!(os << *it)) break;
    }
    return os;
}

std::istream &operator>>(std::istream &is, Tile &t)
{
    char c;
    int v;

    if (is >> c >> v)
    {
        const char *p = strchr(color_letters, c);
        if (p && v >= 1 && v <= 13)
            t = Tile(p - color_letters, v);
        else
            is.setstate(std::ios::failbit);
    }
    return is;
}

std::istream &operator>>(std::istream &is, TileList &vt)
{
    Tile t(0);
    if (is >> t)
    {
        vt.clear();
        vt.push_back(t);
        while (is.peek() == '.')
        {
            (void)is.get();
            if (is >> t)
                vt.push_back(t);
            else
                break;
        }
    }
    return is;
}

std::istream &operator>>(std::istream &is, Table &vvt)
{
    TileList vt;
    if (is >> vt)
    {
        vvt.clear();
        vvt.push_back(vt);
        while (is.peek() == '-')
        {
            (void)is.get();
            if (is >> vt)
                vvt.push_back(vt);
            else
                break;
        }
    }
    return is;
}
