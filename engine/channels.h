#pragma once
#include <channel.h>
#include <lua.h>
#include <sstream>
namespace room_space
{
    struct channel_map;
    struct channels
    {
        channel_map* map;
        std::map< std::string,
                  std::vector< std::pair< uint8_t, std::string > > >
            log;
        channels();
        ~channels();
        channels& operator=(const channels&);
        channel* get_channel(player* pl, std::string name);
        void remove_channel(channel* chan);

        void for_player_channel(player*                               pl,
                                std::function< void(const channel*) > func);

        void for_name_channel(std::string                           name,
                              std::function< void(const channel*) > func);

        void sent(std::string name, std::string mes);

        void resent(const channel* ch);
    };
};