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
	BOOST_SYMBOL_EXPORT void resign(std::string name, std::function< void() > func,
                void* context = nullptr);
	BOOST_SYMBOL_EXPORT void trigger(std::string name,  void* context = nullptr);


    BOOST_SYMBOL_EXPORT void set_context(void* context);

    BOOST_SYMBOL_EXPORT void* new_context();

    BOOST_SYMBOL_EXPORT void* get_context();

    BOOST_SYMBOL_EXPORT void add_data(std::string name, void* data);

    BOOST_SYMBOL_EXPORT const void* get_data(std::string name);

    BOOST_SYMBOL_EXPORT void
    lua_thread_init(std::initializer_list< std::string >   list_name = {},
                    std::initializer_list< lua_CFunction > list_func = {});

    // BOOST_SYMBOL_EXPORT void  lua_add_func();

    BOOST_SYMBOL_EXPORT void run_lua(std::string file);
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