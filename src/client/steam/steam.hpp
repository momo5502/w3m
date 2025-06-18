#pragma once

namespace steam
{
    struct raw_steam_id final
    {
        unsigned int account_id : 32;
        unsigned int account_instance : 20;
        unsigned int account_type : 4;
        int universe : 8;
    };

    typedef union
    {
        raw_steam_id raw;
        unsigned long long bits;
    } steam_id;

#pragma pack(push, 1)
    struct raw_game_id final
    {
        unsigned int app_id : 24;
        unsigned int type : 8;
        unsigned int mod_id : 32;
    };

    typedef union
    {
        raw_game_id raw;
        unsigned long long bits;
    } game_id;
#pragma pack(pop)

    using HSteamPipe = uint64_t;
    using HSteamUser = uint64_t;
}
