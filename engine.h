#pragma once
#include "listener.h"
#include "database_fwd.h"
namespace lsy
{
    class player
    {
      protected:
        port_all&   soc;
        std::string id;
       public:
        player(port_all& soc_);
    };
    class room
    {
        class role
        {
            room& ro;
			player& pl;
 
          public:
            std::string name;
            role(room& ro_, player& pl_);
        };

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
        uint16_t add(player &pl);
		void add(uint16_t role_id,uint16_t channel_id);
		void send(uint16_t from_id,uint16_t to_id);
		void send_channel(uint16_t role_id,uint16_t channel_id);
    };

    class BOOST_SYMBOL_EXPORT engine
    {
    public:
		listener li;
		engine(std::string file);
		void ConnectHandle(port_all& port);
    };
}