#include "database.h"
#include "db_auto.h"
#include "engine.h"
#include "listener.h"
#include "room.h"
#include "socket.h"
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <config.h>
#include <lua_engine.h>
#include <memory>

using namespace config;
using namespace std::string_literals;
thread_local boost::asio::io_service io_service;
lsy::server*                         server_ptr;
/*
int wait_time(lua_State* L)
{
    assert(lua_gettop(L) == 2);
    assert(lua_isnumber(L, 1));
    assert(lua_isstring(L, 2));
    int32_t     time = lua::lua_value(L, 1);
    std::string str  = lua::lua_value(L, 2);
    auto        t    = std::make_shared< boost::asio::deadline_timer >(
        io_service, boost::posix_time::seconds(time));
    auto pt = t.get();
    pt->async_wait([ t, context = lua::get_data(L, "signal_space"), str ](auto
a) {
        lua::trigger(str, context);
    });
    return 0;
}

int add_buttion(lua_State* L)
{
    assert(lua_gettop(L) == 2);
    assert(lua_islightuserdata(L, 1));
    assert(lua_isstring(L, 2));
    assert(lua_isstring(L, 3));
    lsy::player& p     = *(lsy::player*)lua_topointer(L, 1);
    std::string  label = lua::lua_value(L, 2);
    std::string  name  = lua::lua_value(L, 3);
    uint16_t     port  = p->valid_port();
    lsy::buffer  buf{label, name, port};
    p->ports[button_port]->write(buf, []() {});
    auto ptr = p->resign_port(port);
    ptr->OnMessage.connect(
        [& io    = io_service, name,L
          ](lsy::buffer buf) {
            io.post([name, space=lua::get_data(L,"signal_space")]() {
lua::trigger(name, space); });
        });
    return 0;
}

*/
using db_gen::main;

void lsy::run_thread::run()
{
    boost::thread([this]() {
        work.reset(new boost::asio::io_service::work(io_service));
        work->get_io_service().run();
    }).swap(thr);
}

void lsy::run_thread::stop()
{
    work.release();
    thr.join();
}

boost::asio::io_service& lsy::run_thread::get_io_service()
{
    return work->get_io_service();
}

using room_space::room;
void lsy::run_thread::add_room(std::string                 rule_name_,
                               std::vector< lsy::port_all* > vec)
{
    work->get_io_service().post(
        [ rule_name_, vec = std::move(vec) ]() { new room(rule_name_, vec); });
}

lsy::server::server(std::string file)
{
    server_ptr = this;
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml(file, pt);
    threads.resize(pt.get("threads", 1));
    li.add_group(pt.find("engine")->second);
    li.OnConnect.connect([ this, &io = io_service ](lsy::port_all * po) {
        port* p = po->resign_port(login_port);
        p->OnMessage.connect([ this, p, po, &io = io ](buffer buf) {
            std::string id((char*)buf.data());
            std::string passwd((char*)buf.data() + id.size() + 1);
            main.select.bind_once([ passwd, p, po, id, &io = io ](bool sucess) {
                lsy::buffer buf((size_t)2);
                if (sucess)
                {
                    if (passwd == (std::string)main.select[0])
                    {
                        std::cout << "login success" << std::endl;
                        buf.put((uint16_t)0);
                        io.post([po, id]() { new player(po, id); });
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
                io.post([p, buf]() { p->write(buf, []() {}); });
            },
                                  id);
        });
        po->start();
        p->start();
    });
}

void lsy::server::create_room(std::string                 rule_name_,
                              std::vector< lsy::port_all* > vec)
{
    threads[rand() % threads.size()].add_room(rule_name_, vec);
}

void lsy::server::run()
{
    for (auto& thr : threads)
    {
        thr.run();
    }
}


void lsy::run()
{
    boost::asio::io_service::work work(io_service);
    io_service.run();
}