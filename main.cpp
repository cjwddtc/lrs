#include "listener.h"
#include "port.h"
#include "socket.h"
#include <boost/dll.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
#include <thread>
using namespace lsy;
#include "engine.h"
#include <lua_engine.h>
#include <stdlib.h>
#include <time.h>
using namespace std::string_literals;


int main(int n, char* argv[])
{
    server en("server.xml"); /*
     luaL_reg reg;
     reg.name = "test";
     reg.func = test;
     lua::lua_thread_init({reg});
     void* context = lua::new_context();
     lua::set_context(context);
     std::function< void(std::string) > func = [](std::string str) {
         std::cout << "trigger a:" << str << std::endl;
     };
     lua::resign("qwe", func);
     lua::trigger("qwe", "zxc"s);
     lua::declare< int32_t >("af");
     std::string qwe("qqqq");
     lua::add_data("test", (void*)qwe.c_str());
     lua::run_lua("D:\\test.lua");
     lua::trigger("qwe", "fff"s);*/
    run();
    getchar();
    // en.li.close();
    // en.li.join();
}
