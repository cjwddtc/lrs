#include "listener.h"
#include "port.h"
#include "socket.h"
#include <boost/dll.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
#include <thread>
using namespace lsy;
#include "engine.h"
#include <stdlib.h>
#include <time.h>
int main(int n, char* argv[])
{
    std::cout << "test" << std::endl;
    engine en("server.xml");
    std::cout << "asd" << std::endl;
    en.li.join();



    /*
        boost::dll::import<void()>(
            "libwebsocket.so",
            "asd")();
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml("asd.xml", pt);
    listener li;
    li.add_group(pt.find("net_dll")->second);
    li.OnConnect.connect([&li](port_all& p) {
        auto ptr4 = p.resign_port(1234); /*
   std::string str("Hello");
   buffer buf(str.size());
   buf.put((const unsigned char *)str.data(), str.size());
   auto &w=get_writer(*ptr5);
   w.OnWrite.connect([]() {std::cout << "Hello writed" << std::endl; });
   w.send(buf);
        ptr4->OnMessage.connect([&p, &li](buffer a) {
            std::cout << "asd:" << a.get< uint16_t >() << a.get< uint32_t >()
                      << std::endl;
            // std::cout << "qwe:" << << std::endl;
            std::string b(a.begin(), a.end());
            std::cout << "revice:" << b << std::endl;
            p.close();
            li.close();
        });
    }); /*
 listener lii;
 lii.add_group(pt.find("client")->second);
 std::thread thr_cli;
 lii.OnConnect.connect([&lii](port_all &p){
        auto p5=p.resign_port(5);
        p5->OnMessage.connect([&p](buffer a) {
                std::cout << "qwe" << std::endl;
                auto p4=p.resign_port(4);
                auto &w=get_writer(*p4);
                w.OnWrite.connect([]() {std::cout << "response" <<
 std::endl; });
                w.send(a);
        });
 });
 lii.join();
    li.join();*/
}
