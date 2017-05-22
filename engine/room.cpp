#include "room.h"
#include <algorithm>
#include <config.h>
#include <db_auto.h>
#include <lua_engine.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <string>
#include <iostream>
#include <boost/multi_index/mem_fun.hpp>
using db_gen::main;
extern thread_local boost::asio::io_service io_service;
using namespace std::string_literals;


using namespace boost::multi_index;

struct channel
{
	std::pair<std::string, lsy::player*> key;
	const std::string &name()const
	{
		return key.first;
	}
	lsy::player* player()const
	{
		return key.second;
	}
	mutable uint16_t port;
	mutable bool is_enable;
	void set_status(uint16_t st)const
	{
		lsy::buffer buf(name().size() + 3);
		buf.put(st);
		buf.put(name());
		(*player())->ports[config::channel_control]->write(buf, []() {});
	}
};
typedef multi_index_container<
	channel,
	indexed_by<
	hashed_unique<
	member<
	channel, std::pair<std::string, lsy::player*>, &channel::key
	>
	>,
	hashed_non_unique<
	const_mem_fun<
	channel, const std::string &, &channel::name
	>
	>,
	hashed_non_unique<
	const_mem_fun<
	channel, lsy::player *, &channel::player
	>
	>
	>
> animal_multi;


int get_player_channel(lua_State *Ls);

int romm_send_channel(lua_State *Ls);

int channel_enable(lua_State *Ls);

struct channel_map :public animal_multi
{

	void push_player(lua_State *Ls, lsy::player *p)
	{
		lua_pushlightuserdata(Ls, p);
		lua_newtable(Ls);
		lua_pushlightuserdata(Ls, (void *)this);
		lua_pushcclosure(Ls, get_player_channel, 1);
		lua_setfield(Ls, -2, "__index");
		lua_setmetatable(Ls, -2);
	}
	void push_room(lua_State *Ls, lsy::room *r)
	{
		lua_pushlightuserdata(Ls, r);
		lua_newtable(Ls);
		lua_newtable(Ls);
		lua_pushlightuserdata(Ls, (void *)this);
		lua_pushcclosure(Ls, romm_send_channel, 1);
		lua_setfield(Ls, -2, "send");
		lua_pushcclosure(Ls, lua::lua_table_index, 1);
		lua_setfield(Ls, -2, "__index");
		lua_setmetatable(Ls, -2);
	}
	void push_port(lua_State *Ls, lsy::player *p, std::string str)
	{
		auto &iti = this->get<0>();
		auto it = iti.find(std::make_pair(str, p));
		assert(it != iti.end());
		lua_pushlightuserdata(Ls, (void *)(&*it));
		lua_newtable(Ls);
		lua_newtable(Ls);
		lua_pushlightuserdata(Ls, (void *)(&*it));
		lua_pushcclosure(Ls, channel_enable, 1);
		lua_setfield(Ls, -2, "enable");
		lua_pushcclosure(Ls, lua::lua_table_index, 1);
		lua_setfield(Ls, -2, "__index");
		lua_setmetatable(Ls, -2);
	}
};

int get_player_channel(lua_State * Ls)
{
	assert(lua_gettop(Ls) == 2);
	lsy::player *p=(lsy::player *)lua_touserdata(Ls, 1);
	std::string str(lua_tostring(Ls, 2));
	channel_map *c = (channel_map *)lua_touserdata(Ls, lua_upvalueindex(1));
	auto &iti = c->get<0>();
	auto it = iti.find(std::make_pair(str, p));
	if (it == iti.end())
	{
		uint16_t port = (*p)->valid_port();
		auto it=c->insert({ std::make_pair(str, p) ,port });
		it.first->set_status(config::channel_open);
		auto rp = (*p)->resign_port(port);
		rp->start();
		rp->OnMessage.connect([channel=*it.first,c](auto buf) {
			if (channel.is_enable) {
				auto &index = c->get<1>();
				auto its = index.equal_range(channel.name());
				auto it = its.first;
				while (it != its.second)
				{
					if (it->player() != channel.player()) {
						(*it->player())->ports[it->port]->write(buf, []() {});
					}
					++it;
				}
			}
		});
	}
	c->push_port(Ls, p, str);
	return 1;
}

int romm_send_channel(lua_State * Ls)
{
	assert(false);
	return 0;
}

int channel_enable(lua_State * Ls)
{
	bool is= lua_toboolean(Ls, 1);
	channel *c = (channel *)lua_touserdata(Ls, lua_upvalueindex(1));
	c->is_enable = is;
	c->set_status(is ? config::channel_enable : config::channel_disable);
	return 0;
}



lsy::room::room(std::string room_name_, std::vector< player* > vec)
    : room_name(room_name_)
    , context(lua::new_space())
    , count(vec.size())
{
	channel_map *cp = new channel_map();
    // init room rule lua script
    main.get_room_rule.bind_once([ this, &io = io_service, cp](bool have_data) {
        assert(have_data);
        std::string str = main.get_room_rule[0];
        io.post([str, this, cp]() {
			lua::add_data(context, "room"s, [cp, this](auto a) {cp->push_room(a, this); });
            lua::run_lua(str,context);
        });
    },
                                 room_name);
    // init room role
    main.get_room_role.bind([ this, &io = io_service,
                              vec = std::move(vec) , cp](bool is_fin) {
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
            io.post([ this, vec = std::move(vec), cp]() {
				assert(role_names.size() == vec.size());
                std::random_shuffle(role_names.begin(), role_names.end());
                auto name_it = role_names.begin();
                for (auto pl : vec)
                {
                    auto& role  = roles[pl];
                    role.first  = *name_it++;
                    role.second = roles.size() - 1;
                    std::string role_name(role.first);
                    size_t      s = role_name.find_first_of('.');
                    if (s != std::string::npos)
                    {
                        std::string role_ver(role_name.begin() + s + 1,
                                             role_name.end());
                        role_name.resize(s);
                        main.get_role_ver.bind_once(
                            [ this, &io = io_service, ptr = pl, cp](bool is_fin) {
                                if (is_fin)
                                {
                                    std::string filename = main.get_role_ver[0];
                                    io.post([filename, this, ptr, cp]() {
										lua::add_data(context, "player"s, [cp, ptr](auto a) {cp->push_player(a, ptr); });
                                        lua::run_lua(filename, context);
                                    });
                                }
                            },
                            role_name, role_ver);
                    }
                    main.get_role.bind_once(
                        [ this, &io = io_service, ptr = pl, cp](bool is_fin) {
                            if (is_fin)
                            {
                                std::string filename = main.get_role[0];
                                io.post([filename, this, ptr, cp]() {
									printf("player:%p\n", ptr);
									lua::add_data(context, "player"s, [cp, ptr](auto a) {cp->push_player(a,ptr); });
                                    lua::run_lua(filename,context);
                                    count--;
                                    if (count == 0)
                                    {
                                        lua::trigger(config::room_init,context);
                                    }
                                });
                            }
                        },
                        role_name);
                }
            });
        }
    },
                            room_name);
}
