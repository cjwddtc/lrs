#ifndef PORT_H
#define PORT_H
#include <assert.h>
#include <stdint.h>
#include "socket.h"
namespace lsy{
	
class port_all;
class port_write;
class BOOST_SYMBOL_EXPORT port:public assocket
{
    friend class port_write;
	port_all &all;
public:
	uint16_t num;
	port(port_all &all_,uint16_t num_);
    writer& write();
	void close();
	~port();
};

class port_write:public writer
{
    port& soc;
public:
    port_write(port &soc_);
    virtual void send(buffer message);
    virtual ~port_write()=default;
};

class BOOST_SYMBOL_EXPORT port_all {
	friend class port;
    friend class port_write;
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

	std::vector<port *> ports;

	port_all(assocket &soc_);
	
	port *resign_port(uint16_t num);
	void close();
	
	void Message_handle(buffer mes);
};

}
#endif