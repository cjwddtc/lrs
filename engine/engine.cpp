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
                               std::vector< lsy::player* > vec)
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
            main.select.bind_once(
                [ this, passwd, p, po, id, &io = io ](bool sucess) {
                    lsy::buffer buf((size_t)2);
                    if (sucess)
                    {
                        if (passwd == (std::string)main.select[0])
                        {
                            std::cout << "login success" << std::endl;
                            auto p = room_space::get_playing(id);
                            if (p != 0 && !p->is_null())
                            {
                                buf.put((uint16_t)3);
                            }
                            else
                            {
                                buf.put((uint16_t)0);
								auto lcpp=po->resign_port(config::login_comfirm_port);
								lcpp->start();
								lcpp->OnMessage.connect([&io, po,id,p](auto buf) {
                                io.post([po, id, p]() {
                                    new player(po, id);
                                    if (p != 0)
                                    {
                                        p->bind(po);
                                    }
								}); });
                            }
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
                              std::vector< lsy::player* > vec)
{
    std::random_shuffle(vec.begin(), vec.end());
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