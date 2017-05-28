#pragma once
#include "player.h"
#include <boost/signals2.hpp>
#include <channels.h>
#include <map>
#include <room_player.h>
#include <signals.h>
#include <utility>
namespace room_space
{
    class room
    {
        friend class player;
        using signal_s  = room_space::signals;
        using channel_s = room_space::channels;
        std::vector< player >      players;
        signal_s                   sig;
        channel_s                  chs;
        lua_State*                 ls;
        uint8_t                    count;
        std::vector< std::string > role_names;
        std::string                room_name;
		std::string					log;
		std::string replay;

      public:
        void is_dead(uint8_t index, bool is_dead);
        room(std::string room_name, std::vector< lsy::player* > vec);
        signals*  signals();
        channels* channels();
        void wait(int time, std::function< void() > func);
        player* get_player(uint8_t index);
        uint8_t size();
        void sent_public(std::string mes);
        std::string& get_role(uint8_t     index);
        boost::signals2::signal< void() > OnInit;
        void close(uint8_t camp);
		void for_player(int n, std::function<void(player *)> func,int m);
		void for_each_player(int n, std::function<void(player *)> func);
		std::function<void(player *)> load_file(std::string filename);
		uint8_t check();
    };
}