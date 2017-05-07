#pragma once
#include "database_fwd.h"
#include "listener.h"
#include <tuple>
#include <utility>
#include <lua_engine.h>

namespace lsy
{
    class player : private as_contain< port_all >
    {
      protected:
        std::string id;

      public:
        player(port_all* soc);
    };
    class room
    {
		space_class *space;
		uint8_t count;
		typedef std::pair<player&, std::string> role_info;
		//std::map < player&, std::pair<std::string, uint8_t>> roles;
		std::string rule_name;
		room(std::string rule_name,std::vector<role_info> vec,std::function<void()> func);
		/*
        class channel
        {
            std::vector< assocket* > roles;
            uint16_t                 port;
            channel(uint16_t port);

          public:
            void add(role& p);
            void send(buffer buf, std::function< void() > func = []() {});
        };

        std::map< uint16_t, channel > channels;
        std::vector< role* > roles;
        uint16_t add(player& pl);
        void add(uint16_t role_id, uint16_t channel_id);
        void send(uint16_t from_id, uint16_t to_id);
        void send_channel(uint16_t role_id, uint16_t channel_id);*/
    };

    class BOOST_SYMBOL_EXPORT engine
    {
      public:
        listener li;
        engine(std::string file);
        void ConnectHandle(port_all* port);
    };
}
