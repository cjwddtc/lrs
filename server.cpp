#include <algorithm>
#include "server.h"

namespace lsy {
	server_core::server_core()
	{

	}
	void server_core::MessageHandle(port_all &p)
	{
		port &a=*p.resign_port(0);
		a.OnMessage.connect([this,&a](buffer buf) {
			auto end = buf.end();
			auto begin = buf.begin();
			auto p=std::find(begin, end, '\0');
			if (p == end) 
			{
				a.close();
			}
			else 
			{
				
			}
		});
	}
}
