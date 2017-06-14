#pragma once
#include "player.h"
#include <boost/signals2.hpp>
#include <channels.h>
#include <map>
#include <room_player.h>
#include <set>
#include <signals.h>
#include <utility>
namespace room_space
{
    struct group_button
    {
        std::function< void(uint8_t, uint8_t) > on_click;
        std::map< uint8_t, uint8_t >            click_map;
        std::set< uint8_t >            group_mem;
        std::function< void(uint8_t) > on_max;
        void generate(bool is_rand);
        void for_each_player(std::function< void(uint8_t) > func);
    };
    struct room
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
        std::string                log;
        std::stringstream          replay;
        room_space::player*        current_player;
        int                        init_level;

      public:
        ~room();
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
        void for_player(int n, std::function< void(player*) > func, int m);
        void for_each_player(int n, std::function< void(player*) > func);
        std::function< void(player*) > load_file(std::string filename);
        std::array< uint8_t, 32 >    mount_count;
        std::map< uint8_t, uint8_t > condition_map;
		std::function<void(uint8_t)> on_dead;
        void add_condition(uint8_t camp, uint8_t condition);
        bool check();
        void close(uint8_t camp);
        std::map< std::string, group_button > group_data;
        bool add_group_button(player* pl, std::string name,
                              std::function< bool(player*) > func);
        void queue_load(std::string role_name);
        void load_role(std::string role_name);
        group_button* get_group(std::string name);
        void remove_group_button(std::string name);
    };
}