#include "database.h"
#include "engine.h"
#include "listener.h"
#include "socket.h"
#include <boost/property_tree/xml_parser.hpp>


lsy::player::player(port_all& soc_)
    : soc(soc_)
{
	std::cout << "connect" << std::endl;
    port *p=soc.resign_port(0);
    p->OnMessage.connect([this,p](buffer buf){
    	id.assign((char *)buf.data(),buf.size());
    	std::cout.write((char *)buf.data(),buf.size());
    	p->close();
    	port *p_=soc.resign_port(1);
    	const std::string&c_r="create room";
    	uint16_t i=1;
    	for(const std::string&c_r:{"create room","room list","auto match"}){
	    	buffer buf(c_r.size()+2);
	    	buf.put(++i);
	    	buf.put((unsigned char *)c_r.data(),c_r.size());
	    	p_->write(buf,[this](){});
	    }
    });
    soc.get_soc().OnDestroy.connect([this](){std::cout << "desconnect" << std::endl;delete this;});

}
lsy::engine::engine(std::string file)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml(file, pt);
    li.add_group(pt.find("engine")->second);
    li.OnConnect.connect([this](auto& a) { this->ConnectHandle(a); });
}

void lsy::engine::ConnectHandle(port_all& po)
{
    auto p=new player(po);
}
