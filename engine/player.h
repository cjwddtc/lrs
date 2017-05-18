#pragma once
#include <listener.h>
#include <string>
namespace lsy
{
    class player : private as_contain< port_all >
    {
      protected:
      public:
        std::string id;
        player(port_all* soc, std::string id);
        port_all* operator->();
    };
}