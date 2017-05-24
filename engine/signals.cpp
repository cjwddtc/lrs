#include "signals.h"
using namespace room_space;
signal* signals::get_signal(std::string name)
{
    return &map[name];
}

void signal::connect(std::function< void() > func)
{
    sig.connect(func);
}

void signal::trigger()
{
    sig();
}