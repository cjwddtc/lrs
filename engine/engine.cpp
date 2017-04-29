#include "database.h"
#include "db_auto.h"
#include "engine.h"
#include "listener.h"
#include "socket.h"
#include <boost/property_tree/xml_parser.hpp>
using db_gen::main;
lsy::player::player(port_all* soc)
    : as_contain< port_all >(soc)
{
    port* p = ptr->resign_port(0);
    p->OnMessage.connect([this, p](buffer buf) {
        std::string id((char*)buf.data());
        std::string passwd((char*)buf.data() + id.size() + 1);
        main.select.bind(
            [ passwd, p, count = std::make_shared< bool >(0) ](bool have_data) {
                if (have_data)
                {
                    buffer flag(2);
                    flag.put((uint16_t)*count);
					p->write(flag, [p, count]() {if (*count) { p->close(); } });
                }
                else
                {
                    buffer flag(2);
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
    p->start();
}
using db_gen::main;
lsy::engine::engine(std::string file)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml(file, pt);
    li.add_group(pt.find("engine")->second);
    li.OnConnect.connect([this](auto a) { this->ConnectHandle(a); });
}

void lsy::engine::ConnectHandle(port_all* po)
{
    auto p = new player(po);
    po->start();
}
