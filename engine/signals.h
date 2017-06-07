#pragma once
#include <boost/signals2.hpp>
#include <string>
namespace room_space
{
    struct signal
    {
        boost::signals2::signal< void() > sig;
        void connect(std::function< void() > func);
        void trigger();
    };

    struct signals
    {
        std::map< std::string, signal > map;
        signals& operator=(const signals&);
        signal* get_signal(std::string name);
    };
}