#include "database.h"
#include "engine.h"
#include "listener.h"
#include "socket.h"
#include <boost/property_tree/xml_parser.hpp>
lsy::engine::engine(std::string file)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml(file, pt);
    li.add_group(pt.find("network")->second);
    li.OnConnect.connect([this](auto& a) { this->ConnectHandle(a); });
}

void lsy::engine::ConnectHandle(port_all& port)
{
    // port *p=port.resign_port(0);
    // p->OnMessage(std::bind(&lsy::engine::OnLogIn,this,_1));
}

void lsy::engine::OnLogIn(buffer buf)
{
    auto p = std::find(buf.begin(), buf.end(), 0);
}
