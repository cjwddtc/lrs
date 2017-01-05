#include <boost/property_tree/ptree.hpp>
#include <map>
#include "port.h"
#include "socket.h"

namespace lsy{
class listener
{
	boost::signals2::signal<void(port_all &port)> OnConnect;
	std::map<std::string,acceptor *> accs;
	std::map<
	listener();
	void add(boost::property_tree::ptree &pt);
	void add_group(boost::property_tree::ptree &pt);
};
}