#include "room_player.h"
#include <config.h>

#include <memory>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <room.h>
#include <mutex>
extern thread_local boost::asio::io_service io_service;
namespace room_space {
	std::map<std::string, player *> playing_map;
	std::mutex lo;
	void add_playing(std::string str, player *player)
	{
		std::lock_guard<std::mutex> l(lo);
		auto it = playing_map.find(str);
		assert(it == playing_map.end());
		playing_map[str] = player;
	}
	player *get_playing(std::string str)
	{
		std::lock_guard<std::mutex> l(lo);
		auto it = playing_map.find(str);
		if (it != playing_map.end()) {
			return it->second;
		}
		else {
			return nullptr;
		}
		lo.unlock();
	}
}
class null_socket:public lsy::assocket
{

	virtual void start(){}
	virtual void write(lsy::buffer buf, std::function< void() > func = []() {}) {
		func();
	}
	virtual void close() {
		delete this;
	}
};
class null_port_all :public lsy::port_all
{
public:
	null_port_all():lsy::port_all(new null_socket)
	{
		auto p = new lsy::port(this, 0);
		for (auto &a : ports) {
			a = p;
		}
	}
	virtual lsy::port* resign_port(uint16_t num) {
		if (ports[num].valid()) {
			ports[num]->close();
		}
		auto p = new lsy::port(this, num);
		return p;
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

room_space::player::player(room *ro_,lsy::port_all *pl_, uint8_t index_, std::string id_):ro(ro_),pl(nullptr),index(index_),id(id_)
{
	add_playing(id,this);
	rebind(pl_);
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

void room_space::player::sent_public(std::string mes)
{
	pl->ports[config::public_channel]->write(mes, []() {});
}


void keep_doing(int sec, std::function<bool()> func) {

	auto        ptr = std::make_shared< boost::asio::deadline_timer >(
		io_service, boost::posix_time::seconds(sec));
	ptr->async_wait([ptr, func, sec](auto a) {
		if (a == 0) {
			if (func()) {
				keep_doing( sec, func);
			}
		}
	});
}
void room_space::player::rebind(lsy::port_all * pl_)
{
	if (!pl || is_null()) {
		if (pl) {
			pl->close();
		}
		pl = pl_;
		pl->OnDestroy.connect([this]() {
			pl = new null_port_all();
		});
		auto pp = pl->resign_port(config::channel_open);
		pp->OnMessage.connect([this](lsy::buffer) {
			ro->chs.for_player_channel(this, [](const channel *ptr) {
				ptr->open();
				((channel*)ptr)->enable(ptr->is_enable);
			});
		});
		pp->start();
		pp = pl->resign_port(config::player_dead);
		pp->OnMessage.connect([this](lsy::buffer) {
			for (player &pli : ro->players)
			{
				lsy::buffer buf(size_t(2));
				uint8_t fl = pli.is_dead;
				buf.put(&pli.index, 1);
				buf.put(&fl, 1);
				pl->ports[config::player_dead]->write(buf, []() {});
			}
		});
		pp->start();
		pl->resign_port(config::channel_close)->start();
		pl->resign_port(config::public_channel)->start();
		pl->resign_port(config::channel_enable)->start();
		pl->resign_port(config::keep_alive_port)->start();
		pl->resign_port(config::room_init_port)->start();
		pl->ports[config::room_init_port]->write((uint16_t)0, []() {});
		pl->ports[config::room_init_port]->OnMessage.connect([this](auto buf) {
			pl->ports[config::room_init_port]->write((uint16_t)0, []() {});
		});
		auto port = pl->resign_port(config::button_port);
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
		auto p = std::make_shared<bool>(true);
		auto po = pl->ports[config::keep_alive_port];
		po->OnDestroy.connect([p]() {*p = false; });
		keep_doing(3, [p,  po]() {
			if (po.valid()) {
				po->write(lsy::buffer((size_t)0), []() {});
			}
			return *p;
		});
	}
}

bool room_space::player::is_null()
{
	return dynamic_cast<null_port_all*>(pl);
}

void room_space::player::set_camp(uint8_t camp)
{
	this->camp = camp;
}

room_space::player::~player()
{
	if (!is_null()) {
		pl->ports[config::player_dead]->close();
		pl->ports[config::channel_close]->close();
		pl->ports[config::public_channel]->close();
		pl->ports[config::channel_enable]->close();
		pl->ports[config::keep_alive_port]->close();
		
		pl->ports[config::channel_open]->close();
		pl->ports[config::button_port]->close();
		pl->ports[config::room_init_port]->write((uint16_t)1, [pl=pl]() {pl->ports[config::room_init_port]->close(); });
	}

	std::lock_guard<std::mutex> l(lo);
	auto it = playing_map.find(id);
	if (it != playing_map.end()) {
		playing_map.erase(it);
	}
}

room_space::player::player(const player &)
{
	assert(0);
}
