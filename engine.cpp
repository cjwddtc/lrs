//#include "database.h"
#include "engine.h"
#include "listener.h"
#include "socket.h"
#include <boost/property_tree/xml_parser.hpp>

lsy::player::player(port_all* soc)
    : as_contain< port_all >(soc)
{
    port* p = ptr->resign_port(0);
    p->OnMessage.connect([this, p](buffer buf) {
        std::string id((char*)buf.data());
        std::string passwd((char*)buf.data() + id.size() + 1);
        std::cout << id << " login" << std::endl;
        std::cout << "passwd" << passwd << std::endl;
        buffer size(2);
        size.put((uint16_t)3);
        p->write(size, [this, p]() { p->close(); });
        p->OnDestroy.connect([this]() {
            auto p = ptr->resign_port(0);
            p->start();
        });
    });
    p->start();
}
lsy::engine::engine(std::string file)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml(file, pt);
    li.add_group(pt.find("engine")->second);
    li.OnConnect.connect([this](auto a) { this->ConnectHandle(a); });
}

void lsy::engine::ConnectHandle(port_all* po)
{
    po->start();
    std::cout << "qwe" << std::endl;
    auto p = new player(po);
}
