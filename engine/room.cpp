#include "room.h"
#include <algorithm>
#include <config.h>
#include <db_auto.h>
#include <iostream>
#include <lua_bind.h>
#include <string>
using db_gen::main;
extern thread_local boost::asio::io_service io_service;
using namespace std::string_literals;

using namespace room_space;
/*
*/


void room::is_dead(uint8_t index,bool is_dead)
{
	players[index].is_dead = is_dead;
	lsy::buffer buf(size_t(2));
	uint8_t fl = is_dead;
	buf.put(&index, 1);
	buf.put(&fl, 1);
	for (auto p : players)
	{
		(*p)->ports[config::player_dead]->write(buf, []() {});
	}
}

room::room(std::string room_name_, std::vector< lsy::port_all* > vec)
    : room_name(room_name_)
    , count(vec.size())
    , ls(luaL_newstate())
{
    luaL_openlibs(ls);
    lua_put(ls, this);
    setglobal(ls, "room"s);
	uint8_t index=0;
	players.reserve(vec.size());
	for (auto a : vec)
	{
		players.emplace_back(a, index++);
	}
	for (player &pl : players)
	{
		auto pp=pl.pl->resign_port(config::channel_open); 
		pp->OnMessage.connect([this,&pl](lsy::buffer) {
			chs.for_player_channel(&pl,[](const channel *ptr) {
				ptr->open();
				((channel*)ptr)->enable(ptr->is_enable);
			});
		});
		pp->start();
		pp = pl.pl->resign_port(config::player_dead);
		pp->OnMessage.connect([this, &pl](lsy::buffer) {
			for (player &pli : players)
			{
				lsy::buffer buf(size_t(2));
				uint8_t fl = pli.is_dead;
				buf.put(&pli.index, 1);
				buf.put(&fl, 1);
				(*pl)->ports[config::player_dead]->write(buf, []() {});
			}
		});
		pp->start();
	}
    // init room rule lua script
    main.get_room_rule.bind_once([ this, &io = io_service ](bool have_data) {
        assert(have_data);
        std::string str = main.get_room_rule[0];
        io.post([str, this]() { call_file(ls, str); });
    },
                                 room_name);
    // init room role
    main.get_room_role.bind([ this, &io = io_service,
                              vec = std::move(vec) ](bool is_fin) {
        if (!is_fin)
        {
            std::string role_name = main.get_room_role[0];
            int         count     = main.get_room_role[1];
            io.post([this, role_name, count]() {
                int c = count;
                while (c--)
                {
                    role_names.push_back(role_name);
                }
            });
        }
        else
        {
            io.post([ this, vec = std::move(vec) ]() {
                assert(role_names.size() == vec.size());
                std::random_shuffle(role_names.begin(), role_names.end());
                auto name_it = role_names.begin();
                for (auto& pl : players)
                {
                    std::string role_name(*name_it);
                    size_t      s = role_name.find_first_of('.');
                    if (s != std::string::npos)
                    {
                        std::string role_ver(role_name.begin() + s + 1,
                                             role_name.end());
                        role_name.resize(s);
                        main.get_role_ver.bind_once(
                            [ this, &io = io_service, ptr = &pl](bool is_fin) {
                                if (is_fin)
                                {
                                    std::string filename = main.get_role_ver[0];
                                    io.post([filename, this, ptr]() {
                                        call_file(ls, filename, [this, ptr]() {
                                            lua_put(ls, ptr);
                                            return 1;
                                        });
                                    });
                                }
                            },
                            role_name, role_ver);
                    }
                    main.get_role.bind_once(
                        [ this, &io = io_service, ptr = &pl, name = *name_it](bool is_fin) {
                            if (is_fin)
                            {
                                std::string filename = main.get_role[0];
                                io.post([filename, this, ptr, name]() {
									auto rp = ptr->pl->resign_port(config::room_info);
									rp->OnMessage.connect([size = (uint8_t)players.size(), ptr, name](lsy::buffer buf) {
										lsy::buffer bu(name.size() + 2);
										bu.put(name);
										bu.put(&size, 1);
										(*ptr)->ports[config::room_info]->write(bu, []() {});
									});
									rp->start();
                                    call_file(ls, filename, [this, ptr]() {
                                        lua_put(ls, ptr);
                                        return 1;
                                    });
                                    count--;
                                    if (count == 0)
                                    {
                                        sig.get_signal(config::room_init)
                                            ->trigger();
                                    }
                                });
                            }
                        },
                        role_name);
					++name_it;
                }
            });
        }
    },
                            room_name);
}

signals* room::signals()
{
    return &sig;
}

channels* room::channels()
{
    return &chs;
}
#include <memory>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
void room_space::room::wait(int time, std::function<void()> func)
{
	auto        t = std::make_shared< boost::asio::deadline_timer >(
		io_service, boost::posix_time::seconds(time));
	auto pt = t.get();
	pt->async_wait([t, func](auto
		a) {
		func();
	});
}

player * room_space::room::get_player(uint8_t index)
{
	return &players[index];
}
