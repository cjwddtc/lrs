#pragma once
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#include <boost/config.hpp>
#include <channel.h>
#include <channels.h>
#include <room.h>
#include <signals.h>
BOOST_SYMBOL_EXPORT void test();
namespace room_space
{
    BOOST_SYMBOL_EXPORT void setglobal(lua_State* ls, std::string);
    BOOST_SYMBOL_EXPORT void call_file(lua_State* ls, std::string filename,
                                       std::function< int() > push_arg
                                       = []() { return 0; });
    BOOST_SYMBOL_EXPORT void lua_put(lua_State* ls, signals* ptr);
    BOOST_SYMBOL_EXPORT void lua_put(lua_State* ls, signal* ptr);
    BOOST_SYMBOL_EXPORT void lua_put(lua_State* ls, channel* ptr);
    BOOST_SYMBOL_EXPORT void lua_put(lua_State* ls, channels* ptr);
    BOOST_SYMBOL_EXPORT void lua_put(lua_State* ls, room* ptr);
    BOOST_SYMBOL_EXPORT void lua_put(lua_State* ls, player* ptr);
}