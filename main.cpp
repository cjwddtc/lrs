#include "listener.h"
#include "port.h"
#include "socket.h"
#include <boost/dll.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
#include <thread>
using namespace lsy;
#include "engine.h"
#include <lua_bind.h>
#include <lua_engine.h>
#include <stdlib.h>
#include <time.h>
using namespace std::string_literals;


int main(int n, char* argv[])
{
    server en("server.xml");
    en.run();
    run();
    getchar();
    en.li.close();
    en.li.join();
}
