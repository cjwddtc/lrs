#include "listener.h"
#include "port.h"
#include "socket.h"
#include <boost/dll.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
#include <thread>
using namespace lsy;
#include "engine.h"
#include <stdlib.h>
#include <time.h>
int main(int n, char* argv[])
{
	engine en("server.xml");
	getchar();
	en.li.close();
	en.li.join();
}
