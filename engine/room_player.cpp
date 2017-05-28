#include "room_player.h"
#include <config.h>

#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <memory>
#include <mutex>
#include <room.h>
extern thread_local boost::asio::io_service io_service;
const uint16_t                              room_ports[]
    = {config::channel_open,   config::channel_close, config::channel_enable,
       config::role_info,      config::player_dead,   config::room_info,
       config::public_channel, config::button_port};
namespace room_space
{
    std::map< std::string, player* > playing_map;
    std::mutex lo;
    void add_playing(std::string str, player* player)
    {
        std::lock_guard< std::mutex > l(lo);
        auto                          it = playing_map.find(str);
        assert(it == playing_map.end());
        playing_map[str] = player;
    }
    player* get_playing(std::string str)
    {
        std::lock_guard< std::mutex > l(lo);
        auto                          it = playing_map.find(str);
        if (it != playing_map.end())
        {
            return it->second;
        }
        else
        {
            return nullptr;
        }
        lo.unlock();
    }
}
class null_socket : public lsy::assocket
{

    virtual void start()
    {
    }
    virtual void write(lsy::buffer buf, std::function< void() > func = []() {})
    {
        func();
    }
    virtual void close()
    {
        delete this;
    }
};
class null_port : public lsy::port
{
  public:
    null_port(lsy::port_all* p, uint16_t port)
        : port(p, port)
    {
    }
    virtual void close()
    {
        if (num != 0)
        {
            delete this;
        }
    }
};
class null_port_all : public lsy::port_all
{
    null_port* p;

  public:
    null_port_all()
        : lsy::port_all(new null_socket)
    {
        p = new null_port(this, 0);
        for (auto& a : ports)
        {
            a = p;
        }
    }
    virtual lsy::port* resign_port(uint16_t num)
    {
        if (ports[num].valid())
        {
            ports[num].detach();
        }
        ports[num] = new null_port(this, num);
        return ports[num].get();
    }
    ~null_port_all()
    {
        OnDestroy();
        OnDestroy.disconnect_all_slots();
    }
};

lsy::port_all* room_space::player::operator*()
{
    return pl;
}


lsy::port_all* room_space::player::operator->()
{
    return pl;
}

uint8_t room_space::player::get_index()
{
    return index;
}

void check_open(lsy::port_all* po)
{
    for (auto a : room_ports)
    {
        if (!po->ports[a].valid())
        {
            po->resign_port(a)->start();
        }
    }
}

room_space::player::player(room* ro_, lsy::port_all* pl_, uint8_t index_,
                           std::string id_)
    : ro(ro_)
    , pl(pl_)
    , index(index_)
    , id(id_)
    , is_dead(false)
{
    add_playing(id, this);
    bind(pl);
}

void room_space::player::add_button(std::string                    name,
                                    std::function< void(uint8_t) > func)
{
    if (buttons.find(name) == buttons.end())
    {
        pl->mut.lock();
        uint16_t   por  = pl->valid_port();
        lsy::port* port = pl->resign_port(por);
        port->start();
        pl->mut.unlock();
        buttons.emplace(std::make_pair(name, por));
        port->OnMessage.connect([func,&io=io_service](lsy::buffer buf) {
            uint8_t index;
            buf.get(&index, 1);
			io.post([index,func]() {func(index); });
        });
        lsy::buffer buf(name.size() + 3);
        buf.put(por);
        buf.put(name);
        pl->ports[config::button_port]->write(buf, []() {});
    }
    else
    {
        printf("warning:%s alredy exist", name.c_str());
    }
}

void room_space::player::remove_button(std::string name)
{
	auto it = buttons.find(name);
	if (it != buttons.end()) {
		auto p = pl->ports[it->second];
		buttons.erase(it);
		p->write(lsy::buffer((size_t)0), [p]() { p->close(); });
	}
}

void room_space::player::sent_public(std::string mes)
{
    pl->ports[config::public_channel]->write(mes, []() {});
}


void keep_doing(int sec, std::function< bool() > func)
{

    auto ptr = std::make_shared< boost::asio::deadline_timer >(
        io_service, boost::posix_time::seconds(sec));
    ptr->async_wait([ptr, func, sec](auto a) {
        if (a == 0)
        {
            if (func())
            {
                keep_doing(sec, func);
            }
        }
    });
}
void room_space::player::bind(lsy::port_all* pl_)
{
    if (pl != pl_)
    {
        pl->close();
        pl = pl_;
    }
	check_open(pl);
    auto rop = pl->resign_port(config::room_init_port);
    rop->OnMessage.connect([this](auto buf) {
        uint16_t flag;
        buf.get(flag);
        if (flag == 0)
        {
            std::string& rolename = ro->get_role(index);
            lsy::buffer  bu(rolename.size() + 3);
            bu.put(rolename);
            uint8_t size = ro->size();
            bu.put(&size, 1);
            bu.put(&index, 1);
			printf("sending room_info\n");
            pl->ports[config::room_info]->write(bu, []() {});
        }
        else if (flag == 1)
        {
            send_status();
        }
    });
    rop->start();
    rop->write(uint16_t(0), []() {});

    pl->resign_port(config::keep_alive_port)->start();
    auto po = pl->ports[config::keep_alive_port];
    keep_doing(3, [po]() {
        if (po.valid())
        {
            po->write(lsy::buffer((size_t)0), []() {});
            return true;
        }
        else
        {
            return false;
        }
    });

    pl->OnDestroy.connect([this]() { pl = new null_port_all(); });
}

bool room_space::player::is_null()
{
    return dynamic_cast< null_port_all* >(pl);
}

void room_space::player::set_camp(uint8_t camp)
{
    this->camp = camp;
}

bool room_space::player::dead()
{
	return is_dead;
}

room_space::player::~player()
{
    if (!is_null())
    {
        pl->ports[config::player_dead]->close();
        pl->ports[config::channel_close]->close();
        pl->ports[config::public_channel]->close();
        pl->ports[config::channel_enable]->close();
        pl->ports[config::keep_alive_port]->close();

        pl->ports[config::channel_open]->close();
        pl->ports[config::button_port]->close();
        pl->ports[config::room_init_port]->write((uint16_t)1, [pl = pl]() {
            pl->ports[config::room_init_port]->close();
        });
    }

    std::lock_guard< std::mutex > l(lo);
    auto                          it = playing_map.find(id);
    if (it != playing_map.end())
    {
        playing_map.erase(it);
    }
}

room_space::player::player(const player&)
{
    assert(0);
}

void room_space::player::send_status()
{
    check_open(pl);
    ro->chs.for_player_channel(this, [this](const channel* ptr) {
        ptr->open();
		ro->chs.resent(ptr);
		((channel*)ptr)->enable(ptr->is_enable);
    });
    for (player& pli : ro->players)
    {
        lsy::buffer buf(size_t(2));
        uint8_t     fl = pli.is_dead;
        buf.put(&pli.index, 1);
        buf.put(&fl, 1);
        pl->ports[config::player_dead]->write(buf, []() {});
    }
    for (auto& a : buttons)
    {
        lsy::buffer buf(a.first.size() + 3);
        buf.put(a.second);
        buf.put(a.first);
        pl->ports[config::button_port]->write(buf, []() {});
    }
	sent_public(ro->log);
}
