extern "C" {
#include <lauxlib.h>
#include <lua.h>
}
#include <functional>
#include <stdint.h>
#include <string>
#include <boost/config.hpp>

class space_class;

template < class T >
BOOST_SYMBOL_EXPORT void  resign(space_class *sp, std::string name, std::function< void(T) > func);
template < class T >
BOOST_SYMBOL_EXPORT void  trigger(space_class *sp,std::string name, T value);

thread_local extern space_class *space;

BOOST_SYMBOL_EXPORT space_class * new_space();

BOOST_SYMBOL_EXPORT void  lua_init();

BOOST_SYMBOL_EXPORT void  lua_thread_init();

//BOOST_SYMBOL_EXPORT void  lua_add_func();

BOOST_SYMBOL_EXPORT void  run_lua(std::string file);