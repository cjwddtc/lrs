#include <channel.h>
using namespace room_space;
room_space::channel::channel(std::string str, room_space::player * pl, uint16_t port_):key(str,pl),port(port_)
{
}
const std::string& channel::name() const
{
    return key.first;
}
player* channel::player() const
{
    return key.second;
}

void room_space::channel::open()const
{
	lsy::buffer buf(name().size() + 3);
	buf.put(port);
	buf.put(name());
	(*player())->ports[config::channel_open]->write(buf, []() {});
}

void channel::enable(bool is_enable)
{
	this->is_enable = is_enable;
	lsy::buffer buf(name().size() + 3);
	buf.put((uint16_t)is_enable);
	buf.put(name());
	printf("enable:%d\n", is_enable);
	(*player())->ports[config::channel_enable]->write(buf, []() {});
}

room_space::channel::~channel()
{
	(*player())->ports[port]->close();
}
