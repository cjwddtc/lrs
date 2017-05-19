#include "listener.h"
#include "message.h"
#include <boost/dll/import.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>

typedef lsy::socket_getter* acceptor_fun();

void lsy::listener::add(std::string name, boost::property_tree::ptree& pt)
{
    auto p = boost::dll::import< acceptor_fun >(
        pt.get< std::string >("lib_path"), pt.get("listen", "listen"));

    auto  ptr   = p();
    auto& value = accs[name];
    value.first = ptr;
    ptr->OnNewSocket.connect(
        [ this, is_stream = pt.get< bool >("is_stream") ](assocket * p) {
            p->OnMessage.connect([](auto buf) {
            });
            if (is_stream)
            {
                OnConnect(new port_all(new message_socket(p)));
            }
            else
            {
                OnConnect(new port_all(p));
            }
        });
    ptr->start(pt, value.second);
}

void lsy::listener::add_group(boost::property_tree::ptree& pt)
{
    for (auto pt_ : pt)
    {
        add(pt_.first, pt_.second);
    }
}

lsy::listener::listener()
{
}

void lsy::listener::join()
{
    for (auto& a : accs)
    {
        a.second.second.join();
    }
}

void lsy::listener::close()
{
    for (auto& a : accs)
    {
        a.second.first->close();
    }
}
