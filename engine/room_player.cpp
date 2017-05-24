#include "room_player.h"
#include <config.h>

class null_socket:public lsy::assocket
{

	virtual void start(){}
	virtual void write(lsy::buffer buf, std::function< void() > func = []() {}) {
		func();
	}
	virtual void close() {
		OnDestroy();
		delete this;
	}
};

lsy::port_all * room_space::player::operator*()
{
	return pl;
}


lsy::port_all * room_space::player::operator->()
{
	return pl;
}

uint8_t room_space::player::get_index()
{
	return index;
}

room_space::player::player(lsy::port_all *pl_, uint8_t index_):pl(pl_),index(index_)
{
	pl->resign_port(config::channel_close)->start();
	pl->resign_port(config::channel_enable)->start();
	auto port=pl->resign_port(config::button_port);
	port->start();
	port->OnMessage.connect([this](auto buf) {
		for (auto &a : buttons)
		{
			lsy::buffer buf(a.first.size() + 3);
			buf.put(a.second);
			buf.put(a.first);
			pl->ports[config::button_port]->write(buf, []() {});
		}
	});
	pl->OnDestroy.connect([this]() {pl = new lsy::port_all(new null_socket); });
}

void room_space::player::add_button(std::string name, std::function<void(uint8_t)> func)
{
	if (buttons.find(name) == buttons.end()) {
		pl->mut.lock();
		uint16_t por = pl->valid_port();
		lsy::port *port = pl->resign_port(por);
		port->start();
		pl->mut.unlock();
		buttons.insert(std::make_pair(name, por));
		port->OnMessage.connect([func](lsy::buffer buf) {
			uint8_t index;
			buf.get(&index, 1);
			func(index);
		});
		lsy::buffer buf(name.size() + 3);
		buf.put(por);
		buf.put(name);
		pl->ports[config::button_port]->write(buf, []() {});
	}
	else {
		printf("warning:%s alredy exist", name.c_str());
	}
}

void room_space::player::remove_button(std::string name)
{
	auto p = pl->ports[buttons[name]];
	p->write(lsy::buffer((size_t)0), [p]() {p->close(); });
}
