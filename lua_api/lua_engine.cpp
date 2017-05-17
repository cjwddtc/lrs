#include "lua_engine.h"
#include <boost/variant.hpp>
#include <functional>
#include <map>
#include <vector>
namespace lua
{
    thread_local lua_State* Ls = nullptr;

    void lua_push(std::string str)
    {
        lua_pushstring(Ls, str.c_str());
    }

    void lua_push(int32_t n)
    {
        lua_pushinteger(Ls, n);
    }

    class signal
    {
        typedef boost::variant<int, std::function< void() > >
            function;
        class function_run : public boost::static_visitor<>
        {

          public:
			  void operator()(int func) const;
            void operator()(std::function< void() > func) const
            {
                func();
            }
        };
        std::vector< function > funcs;

      public:
        static void run(function func)
        {
            boost::apply_visitor(function_run(), func);
        }
        void resign(function func)
        {
            funcs.push_back(func);
        }
        void trigger()
        {
            for (function& f : funcs)
            {
                run(f);
            }
        }
    };

    class space_class
        : public std::map< std::string,signal >
    {
      public:
        int id;
        space_class()
        {
            lua_newtable(Ls);
            id = luaL_ref(Ls, 1);
        }
        void get_table()
        {
            lua_getglobal(Ls, "funcmap");
        }
    };

    thread_local space_class* space;

    void resign(std::string name, std::function< void() > func, void* sp)
    {
        if (sp == nullptr)
        {
            sp = (void*)space;
        }
        (*(space_class*)sp)[name].resign(func);
    }

    void trigger(std::string name, void* sp)
    {
        if (sp != nullptr)
        {
            set_context(sp);
        }
        (*space)[name].trigger();
    }

    int lua_resign(lua_State* L)
    {
        assert(lua_gettop(L) == 2);
        assert(lua_isfunction(L, 2));
        assert(lua_isstring(L, 1));
        size_t      size;
        const char* name_ = lua_tolstring(L, 1, &size);
        std::string name(name_, size);
		space->get_table();
		lua_pushvalue(Ls,2);
		int n=luaL_ref(Ls, -2);
		(*space)[name].resign(n);
		lua_pop(Ls, 1);
        return 0;
    }

    int lua_trigger(lua_State* L)
    {
        assert(lua_gettop(L) == 1);
		(*space)[lua_value(L, 1)].trigger();
        return 0;
    }

    void set_context(void* context)
    {
        if (space != (space_class*)context)
        {
            space = (space_class*)context;
            lua_rawgeti(Ls, 1, space->id);
            lua_setglobal(Ls, "funcmap");
        }
    }

    void* new_context()
    {
        return (void*)new space_class();
    }


    void* get_context()
    {
        return (void*)space;
    }

    void add_data(std::string name, void* data)
    {
        space->get_table();
        lua_pushlightuserdata(Ls, data);
        lua_setfield(Ls, -2, name.c_str());
        lua_pop(Ls, 1);
    }

    const void* get_data(std::string name)
    {
        space->get_table();
        lua_getfield(Ls, -1, name.c_str());
        auto ptr = lua_topointer(Ls, -1);
        lua_pop(Ls, 1);
        return ptr;
    }

    void lua_thread_init(std::initializer_list< std::string >   list_name,
                         std::initializer_list< lua_CFunction > list_func)
    {
        Ls = luaL_newstate();
        lua_register(Ls, "trigger", lua_trigger);
        lua_register(Ls, "resign", lua_resign);
        lua_newtable(Ls);
        assert(list_name.size() == list_func.size());
        for (size_t i = 0; i < list_name.size(); i++)
        {
            lua_register(Ls, list_name.begin()[i].c_str(),
                         list_func.begin()[i]);
        }
        luaL_openlibs(Ls);
    }

    lua_value::lua_value(lua_State* L_, int index_)
        : L(L_)
        , index(index_)
    {
    }
    lua_value::operator std::string()
    {
        assert(lua_isstring(L, index));
        size_t      size;
        const char* name = lua_tolstring(L, index, &size);
        return std::string(name, size);
    }
    lua_value::operator int32_t()
    {
        assert(lua_isnumber(L, index));
        return lua_tointeger(L, index);
    }


    void BOOST_SYMBOL_EXPORT run_lua(std::string file)
    {
        luaL_dofile(Ls, file.c_str());
    }
	void signal::function_run::operator()(int func) const
	{
		space->get_table();
		lua_rawgeti(Ls, -1, func);
		assert(lua_isfunction(Ls, lua_gettop(Ls)));
		lua_call(Ls, 1, 0);
		lua_pop(Ls, 1);
	}
}