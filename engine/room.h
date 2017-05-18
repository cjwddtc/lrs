#pragma once
#include <map>
#include <utility>
#include "player.h"
namespace lsy{
	typedef std::pair< player&, std::string > role_info;
    class room
    {
        void*   context;
        uint8_t count;
        std::map< player*, std::pair< std::string, uint8_t > > roles;
        std::string rule_name;

      public:
        room(std::string rule_name, std::vector< role_info > vec);
    };
}