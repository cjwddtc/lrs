#ifndef PORT_H
#define PORT_H
#include <assert.h>
#include <stdint.h>
#include "socket.h"
namespace lsy{
	
class port_all;
class port:public assocket
{
	port_all &all;
	uint16_t num;
public:
	port(port_all &all_,uint16_t num_);
	boost::signals2::signal<void(size_t)> *send(buffer buf);
	void close();
	~port();
};

class port_all {
	friend class port;
	assocket &soc;
	buffer buf;
	buffer head;
	bool is_head;
public:

	class port_using:public std::exception
	{
	public:
		uint16_t port;
		port_using(uint16_t port);
	};

	port *ports[65536];

	port_all(assocket &soc_);
	
	port *resign_port(uint16_t num);
	void close();
	
	void Message_handle(buffer mes);
};

}
#endif