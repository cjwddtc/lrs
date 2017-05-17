#include "database.h"
#include "db_auto.h"
#include "engine.h"
#include "listener.h"
#include "socket.h"
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <config.h>
#include <lua_engine.h>
#include <memory>

using db_gen::main;
using namespace config;
using namespace std::string_literals;
thread_local boost::asio::io_service io_service;

int wait_time(lua_State* L)
{
    assert(lua_gettop(L) == 2);
    assert(lua_isnumber(L, 1));
    assert(lua_isstring(L, 2));
    int32_t     time = lua::lua_value(L, 1);
    std::string str  = lua::lua_value(L, 2);
    lua::declare< int32_t >(str);
    auto t = std::make_shared< boost::asio::deadline_timer >(
        io_service, boost::posix_time::seconds(time));
    auto pt = t.get();
    pt->async_wait([ t, context = lua::get_context(), str ](auto a) {
        lua::trigger(str, 0, context);
    });
    return 0;
}

int add_buttion(lua_State* L)
{
    assert(lua_gettop(L) == 2);
    assert(lua_islightuserdata(L, 1));
    assert(lua_isstring(L, 2));
    assert(lua_isstring(L, 2));
    lsy::player& p     = *(lsy::player*)lua_topointer(L, 1);
    std::string  label = lua::lua_value(L, 2);
    std::string  name  = lua::lua_value(L, 3);
    uint16_t     port  = p->valid_port();
    lsy::buffer  buf{label, name, port};
    p->ports[button_port]->write(buf, []() {});
    auto ptr = p->resign_port(port);
    lua::declare< int32_t >(name);
    ptr->OnMessage.connect(
        [& io    = io_service, name,
         context = lua::get_context() ](lsy::buffer buf) {
            io.post([name, context]() { lua::trigger(name, 0, context); });
        });
    return 0;
}


lsy::port_all* lsy::player::operator->()
{
    return ptr;
}

lsy::room::room(std::string rule_name_, std::vector< role_info > vec)
    : rule_name(rule_name_)
    , context(lua::new_context())
    , count(vec.size())
{
    lua::set_context(context);
    lua::declare< int32_t >(room_init);
    main.get_rule.bind([ this, &io = io_service ](bool is_fin) {
        if (!is_fin)
        {
            std::string filename = main.get_rule[0];
            io.post([filename, this]() {
                lua::set_context(context);
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
            main.get_role_ver.bind([ this, &io = io_service ](bool is_fin) {
                if (!is_fin)
                {
                    std::string filename = main.get_role_ver[0];
                    io.post([filename, this]() {
                        lua::set_context(context);
                        lua::run_lua(filename);
                    });
                }
            },
                                   rf.second, role);
        }
        main.get_role.bind([ this, &io = io_service ](bool is_fin) {
            if (!is_fin)
            {
                std::string filename = main.get_role[0];
                io.post([filename, this]() {
                    lua::set_context(context);
                    lua::run_lua(filename);
                    count--;
                    if (count == 0)
                    {
                        lua::trigger(room_init, 0);
                    }
                });
            }
        },
                           rf.second);
    }
}

/*
void lsy::room::add_role(player &p, std::string name, std::string version)
{
        roles.push_back(role(*this,p,roles.size(),name,version));
        main.get_role.bind([this](bool is_fin) {
                if (is_fin) {
                        std::string filename = main.get_role[0];
                        soc_get->post([filename, this]() {
                                set_lua_State(L);
                                run_lua(filename);
                        });
                }}, name);
        if (version != "") {
                main.get_role_ver.bind([this](bool is_fin) {
                        if (is_fin) {
                                std::string filename = main.get_role_ver[0];
                                soc_get->post([filename, this]() {
                                        set_lua_State(L);
                                        run_lua(filename);
                                        count--;
                                        if (count == 0) {
                                                OnReady();
                                        }
                                });
                        }}, name, version);
        }
        //main.
}*/
lsy::player::player(port_all* soc)
    : as_contain< port_all >(soc)
{ /*
     port* p = ptr->resign_port(0);
     p->OnMessage.connect([this, p](buffer buf) {
         std::string id((char*)buf.data());
         std::string passwd((char*)buf.data() + id.size() + 1);
         main.select.bind(
             [ passwd, p, count = std::make_shared< bool >(0) ](bool have_data)
     {
                 if (have_data)
                 {
                     buffer flag((size_t)2);
                     flag.put((uint16_t)*count);
                     p->write(flag, [p, count]() {
                         if (*count)
                         {
                             p->close();
                         }
                     });
                 }
                 else
                 {
                     buffer flag((size_t)2);
                     std::cout << passwd << std::endl;
                     std::cout << (std::string)main.select[0] << std::endl;
                     if (passwd == (std::string)main.select[0])
                     {
                         *count = 1;
                         std::cout << "login success" << std::endl;
                     }
                 }
             },
             id);
     });
     p->start();*/
}
using db_gen::main;
lsy::engine::engine(std::string file)
{
    lua::lua_thread_init({"wait"s}, {wait_time});
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml(file, pt);
    li.add_group(pt.find("engine")->second);
    li.OnConnect.connect([ this, &io = io_service ](lsy::port_all * po) {
        io.post([this, po]() {
            port* p = po->resign_port(login_port);
            p->OnMessage.connect([this, p, po](buffer buf) {
                std::string id((char*)buf.data());
                std::string passwd((char*)buf.data() + id.size() + 1);
                main.select.bind_once([passwd, p, po](bool sucess) {
                    lsy::buffer buf((size_t)2);
                    if (sucess)
                    {
                        std::cout << passwd << std::endl;
                        std::cout << (std::string)main.select[0] << std::endl;
                        if (passwd == (std::string)main.select[0])
                        {
                            std::cout << "login success" << std::endl;
                            buf.put((uint16_t)0);
                            new player(po);
                        }
                        else
                        {
                            buf.put((uint16_t)1);
                        }
                    }
                    else
                    {
                        buf.put((uint16_t)2);
                    }
                    p->write(buf, []() {});
                });
                po->start();
                p->start();
            });
            // auto p = new player(po);
            // using role_info = lsy::room::role_info;
            // new room("test"s, { role_info(*p,"test"s), });
        });
    });
}

void lsy::engine::ConnectHandle(port_all* po)
{
}

void lsy::run()
{
    boost::asio::io_service::work w(io_service);
    io_service.run();
}
