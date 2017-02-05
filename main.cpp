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
{ /*
     engine en("server.xml");
     en.li.join();
     */
    socket_getter* ptr = boost::dll::import< socket_getter*() >(
        "libtcp.so", "tcp_listner_socket_getter")();
    boost::property_tree::ptree pt;
    std::thread                 thr;
    ptr->start(pt, thr);
    thr.join();
}
