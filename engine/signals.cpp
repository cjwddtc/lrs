#include "signals.h"
using namespace room_space;
signals & room_space::signals::operator=(const signals &)
{
	assert(false);
	return *this;
}
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