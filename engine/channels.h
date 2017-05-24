#pragma once
#include <channel.h>
#include <lua.h>
namespace room_space
{
    struct channel_map;
    struct channels
    {
        channel_map* map;
        channels();
        ~channels();
        channel* get_channel(player* pl, std::string name);
		void remove_channel(channel *chan);
		void for_player_channel(player *pl,std::function<void(const channel *)> func);
        void sent(std::string name, std::string mes);
    };
};