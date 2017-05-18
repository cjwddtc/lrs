#pragma once
#include "player.h"
#include <map>
#include <utility>
namespace lsy
{
    class room
    {
        void*   context;
        uint8_t count;
        std::map< player*, std::pair< std::string, uint8_t > > roles;
        std::vector< std::string > role_names;
        std::string                room_name;

      public:
        room(std::string room_name, std::vector< player* > vec);
    };
}