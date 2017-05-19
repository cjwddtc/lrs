#include "lua_engine.h"
#include <boost/variant.hpp>
#include <functional>
#include <map>
#include <vector>
#define get_space(s) (*(space_class*)s)
namespace lua
{

    class signal
    {
        typedef boost::variant< std::pair<lua_State*,int>, std::function< void() > > function;
        class function_run : public boost::static_visitor<>
        {

          public:
            void operator()(std::pair<lua_State*, int> func) const;
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


	void add_data(lua_State *L, std::string name, void* data)
	{
		lua_getglobal(L, "global_map");
		lua_pushlightuserdata(L, data);
		lua_setfield(L, -2, name.c_str());
		lua_pop(L, 1);
	}

	void* get_data(lua_State *L, std::string name)
	{
		lua_getglobal(L, "global_map");
		lua_getfield(L, -1, name.c_str());
		auto p = lua_touserdata(L, -1);
		lua_pop(L, 1);
		return p;
	}

    class space_class : public std::map< std::string, signal >
    {
      public:
		  lua_State* Ls;
		  space_class();
    };

    void resign(std::string name, std::function< void() > func, void* sp)
    {
		get_space(sp)[name].resign(func);
    }

    void trigger(std::string name, void* sp)
    {
		get_space(sp)[name].trigger();
    }

    int lua_resign(lua_State* L)
    {
        assert(lua_gettop(L) == 2);
        assert(lua_isfunction(L, 2));
        assert(lua_isstring(L, 1));
        size_t      size;
        const char* name_ = lua_tolstring(L, 1, &size);
        std::string name(name_, size);
		lua_getglobal(L, "global_map");
        lua_pushvalue(L, 2);
        int n = luaL_ref(L, -2);
		auto space = get_data(L, "signal_space");
		get_space(space)[name].resign(std::make_pair(L,n));
        lua_pop(L, 1);
        return 0;
    }

    int lua_trigger(lua_State* L)
    {
        assert(lua_gettop(L) == 1);
		auto space = get_data(L, "signal_space");
        get_space(space)[lua_value(L, 1)].trigger();
        return 0;
    }

	std::vector<std::pair<std::string, lua_CFunction>> reg_funcs = {std::make_pair("resign",lua_resign),std::make_pair("trigger",lua_trigger) };

    void* new_space()
    {
        return (void*)new space_class();
    }
	
    void add_data(void *space,std::string name, void* data)
    {
		add_data(get_space(space).Ls,std::move(name), data);
    }

    void* get_data(void *space,std::string name)
    {
        return get_data(get_space(space).Ls, std::move(name));
    }

	BOOST_SYMBOL_EXPORT void add_func(std::string name, lua_CFunction func)
	{
		reg_funcs.push_back(std::make_pair(name, func));
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


    void BOOST_SYMBOL_EXPORT run_lua(std::string file,void *space)
    {
        luaL_dofile(get_space(space).Ls, file.c_str());
    }
    void signal::function_run::operator()(std::pair<lua_State*, int> func) const
    {
		lua_getglobal(func.first, "global_map");
        lua_rawgeti(func.first, -1, func.second);
        assert(lua_isfunction(func.first, lua_gettop(func.first)));
        lua_call(func.first, 0, 0);
        lua_pop(func.first, 1);
    }
	space_class::space_class() :Ls(luaL_newstate())
	{
		for (auto &a : reg_funcs)
		{
			lua_register(Ls, a.first.c_str(), a.second);
		}
		lua_newtable(Ls);
		luaL_openlibs(Ls);
		lua_pushlightuserdata(Ls, (void*)this);
		lua_setfield(Ls, 1, "signal_space");
		lua_setglobal(Ls, "global_map");
	}
}