#include "room.h"
#include <lua_engine.h>
#include <db_auto.h>
#include <config.h>
using db_gen::main;
extern thread_local boost::asio::io_service io_service;
using namespace std::string_literals;
lsy::room::room(std::string rule_name_, std::vector< role_info > vec)
    : rule_name(rule_name_)
    , context(lua::new_context())
    , count(vec.size())
{
    lua::set_context(context);
    main.get_rule.bind_once([ this, &io = io_service ](bool is_fin) {
        if (is_fin)
        {
            std::string filename = main.get_rule[0];
            io.post([filename, this]() {
                lua::set_context(context);
				lua::add_data("room"s, this);
                lua::run_lua(filename);
            });
        }
    },
                            rule_name_);
    for (role_info& rf : vec)
    {
        auto& role  = roles[&rf.first];
        role.first  = rf.second;
        role.second = roles.size() - 1;
        size_t s    = rf.second.find_first_of('.');
        if (s != std::string::npos)
        {
            std::string role(rf.second.begin() + s + 1, rf.second.end());
            rf.second.resize(s);
            main.get_role_ver.bind_once(
                [ this, &io = io_service ,ptr=&rf.first](bool is_fin) {
                    if (is_fin)
                    {
                        std::string filename = main.get_role_ver[0];
                        io.post([filename, this,ptr]() {
                            lua::set_context(context);
							lua::add_data("room"s, this);
							lua::add_data("player"s, ptr);
                            lua::run_lua(filename);
                        });
                    }
                },
                rf.second, role);
        }
        main.get_role.bind_once([ this, &io = io_service, ptr = &rf.first](bool is_fin) {
            if (is_fin)
            {
                std::string filename = main.get_role[0];
                io.post([filename, this,ptr]() {
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
                                rf.second);
    }
}