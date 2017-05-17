#pragma once
#include "database_fwd.h"
#include "listener.h"
#include <tuple>
#include <utility>

namespace lsy
{
    class player : private as_contain< port_all >
    {
      protected:
        std::string id;

      public:
        player(port_all* soc);
        port_all* operator->();
    };

    class room
    {
        void*   context;
        uint8_t count;
        std::map< player*, std::pair< std::string, uint8_t > > roles;
        std::string rule_name;

      public:
        typedef std::pair< player&, std::string > role_info;
        room(std::string rule_name, std::vector< role_info > vec);
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
    BOOST_SYMBOL_EXPORT void run();
}
