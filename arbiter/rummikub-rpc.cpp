#include "rummikub.h"
#include <curl/curl.h>
#include <sstream>

static size_t ostream_writer(void *ptr, size_t size, size_t nmemb, void *stream)
{
    ((std::ostream*)stream)->write((const char*)ptr, size*nmemb);
    return size*nmemb;
}

static std::string build_query_string(const GameState &gs)
{
    std::ostringstream oss;
    oss << "yourTiles=" << gs.player_tiles[gs.next_player]
        << "&table=" << gs.table_tiles
        << "&poolTiles=" << gs.pool_tiles.size()
        << "&opponentsTiles=" << gs.player_tiles[(gs.next_player + 1)%4].size()
        << '.'                << gs.player_tiles[(gs.next_player + 2)%4].size()
        << '.'                << gs.player_tiles[(gs.next_player + 3)%4].size();
    return oss.str();
}

bool rpc_move(const char *base_url, bool post, int timeout, const GameState &gs,
              std::string &query, std::string &response)
{
    std::ostringstream oss;
    std::string url = base_url;
    query = build_query_string(gs);
    if (!post) url += '?' + query;
    CURL *ch = curl_easy_init();
    if (!ch) return false;
    curl_easy_setopt(ch, CURLOPT_URL, url.c_str());
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, ostream_writer);
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, &(std::ostream&)oss);
    curl_easy_setopt(ch, CURLOPT_TIMEOUT, timeout);
    if (post)
    {
        curl_easy_setopt(ch, CURLOPT_POST, (long)1);
        curl_easy_setopt(ch, CURLOPT_POSTFIELDS, query.c_str());
        curl_easy_setopt(ch, CURLOPT_POSTFIELDSIZE, (long)query.size());
    }

    long code;
    bool ok = (curl_easy_perform(ch) == 0) &&
              (curl_easy_getinfo(ch, CURLINFO_RESPONSE_CODE, &code)) == 0 &&
              oss.good() && code == 200;
    curl_easy_cleanup(ch);
    if (ok) response = oss.str();
    return ok;
}
