#ifndef LISTNER_H
#define LISTNER_H
#include <boost/property_tree/ptree.hpp>
#include <map>
#include "port.h"
#include "socket.h"
#include "listener.h"

namespace lsy{
class listener
{
	std::map<std::string,std::pair<acceptor *,std::thread>> accs;
public:
	boost::signals2::signal<void(port_all &port)> OnConnect;
	listener();
	void add(std::string name,boost::property_tree::ptree &pt);
	void add_group(boost::property_tree::ptree &pt);
	void join();
	void close();
};
}
#endif