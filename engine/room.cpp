#include "room.h"
#include <algorithm>
#include <config.h>
#include <db_auto.h>
#include <lua_engine.h>
using db_gen::main;
extern thread_local boost::asio::io_service io_service;
using namespace std::string_literals;
lsy::room::room(std::string room_name_, std::vector< player* > vec)
    : room_name(room_name_)
    , context(lua::new_context())
    , count(vec.size())
{
    lua::set_context(context);
    // init room rule lua script
    main.get_room_rule.bind_once([ this, &io = io_service ](bool have_data) {
        assert(have_data);
        std::string str = main.get_room_rule[0];
        io.post([str, this]() {
            lua::set_context(context);
            lua::add_data("room"s, this);
            lua::run_lua(str);
        });
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
                            [ this, &io = io_service, ptr = &pl ](bool is_fin) {
                                if (is_fin)
                                {
                                    std::string filename = main.get_role_ver[0];
                                    io.post([filename, this, ptr]() {
                                        lua::set_context(context);
                                        lua::add_data("room"s, this);
                                        lua::add_data("player"s, ptr);
                                        lua::run_lua(filename);
                                    });
                                }
                            },
                            role_name, role_ver);
                    }
                    main.get_role.bind_once(
                        [ this, &io = io_service, ptr = &pl ](bool is_fin) {
                            if (is_fin)
                            {
                                std::string filename = main.get_role[0];
                                io.post([filename, this, ptr]() {
                                    lua::set_context(context);
                                    lua::add_data("room"s, this);
                                    lua::add_data("player"s, ptr);
                                    lua::run_lua(filename);
                                    count--;
                                    if (count == 0)
                                    {
                                        lua::trigger(config::room_init);
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