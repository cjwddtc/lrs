#pragma once
#include "listener.h"
namespace lsy {
	class server_core :listener
	{
		server_core();
		void MessageHandle(port_all &p);
	};
}