extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#include <boost/config.hpp>
#include <functional>
#include <stdint.h>
#include <string>
namespace lua
{
    BOOST_SYMBOL_EXPORT void resign(std::string             name,
                                    std::function< void() > func, void* space);
    BOOST_SYMBOL_EXPORT void trigger(std::string name, void* space);

    BOOST_SYMBOL_EXPORT void* new_space();

    BOOST_SYMBOL_EXPORT lua_State* get_state(void* space);

    BOOST_SYMBOL_EXPORT void
    add_data(void* ptr, std::string name,
             std::function< void(lua_State* Ls) > func);

    BOOST_SYMBOL_EXPORT void add_data(lua_State* L, std::string name,
                                      void* data);

    BOOST_SYMBOL_EXPORT void* get_data(lua_State* L, std::string name);

    BOOST_SYMBOL_EXPORT void add_func(std::string name, lua_CFunction func);

    BOOST_SYMBOL_EXPORT void run_lua(std::string file, void* space);

    BOOST_SYMBOL_EXPORT int lua_table_index(lua_State* Ls);

    class BOOST_SYMBOL_EXPORT lua_value
    {
        lua_State* L;
        int        index;

      public:
        lua_value(lua_State* L, int index);
        operator std::string();
        operator int32_t();
    };
}