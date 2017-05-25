#pragma once
#include "player.h"
#include <channels.h>
#include <map>
#include <signals.h>
#include <utility>
#include <room_player.h>
namespace room_space
{
    class room
    {
		friend class player;
        using signal_s  = room_space::signals;
        using channel_s = room_space::channels;
		std::vector<player> players;
        signal_s   sig;
        channel_s  chs;
        lua_State* ls;
        uint8_t    count;
        std::vector< std::string > role_names;
        std::string                room_name;

      public:
		  void is_dead(uint8_t index, bool is_dead);
		  room(std::string room_name, std::vector< lsy::player* > vec);
        signals*  signals();
        channels* channels();
		void wait(int time, std::function<void()> func);
		player *get_player(uint8_t index);
		uint8_t size();
		void sent_public(std::string mes);
		void close(uint8_t camp);
    };
}