#include "lua_engine.h"
#include <boost/variant.hpp>
#include <functional>
#include <map>
#include <vector>

thread_local lua_State* Ls;

void lua_push(std::string str)
{
    lua_pushstring(Ls, str.c_str());
}

void lua_push(int32_t n)
{
    lua_pushinteger(Ls, n);
}

template < class T >
class signal
{
    typedef boost::variant< lua_CFunction, std::function< void(T) > > function;
    class function_run : public boost::static_visitor<>
    {
        T value;

      public:
        function_run(T value_)
            : value(value_)
        {
        }
        void operator()(lua_CFunction func) const
        {
            lua_pushcfunction(Ls, func);
            lua_push(value);
            lua_call(Ls, 1, 0);
        }
        void operator()(std::function< void(T) > func) const
        {
            func(value);
        }
    };
    std::vector< function > funcs;

  public:
    static void run(function func, T& a)
    {
        boost::apply_visitor(function_run(a), func);
    }
    template < class T >
    void resign(T func)
    {
        funcs.push_back(func);
    }
    void trigger(T a)
    {
        for (function& f : funcs)
        {
            run(f, a);
        }
    }
};

class space_class :public std::map< std::string,
	boost::variant< signal< std::string >, signal< int32_t > > > {};

thread_local  space_class *space;

template < class T >
void resign(space_class *sp,std::string name, std::function< void(T) > func)
{
    boost::get< signal< T > >((*sp)[name]).resign(func);
}
template < class T >
void trigger(space_class *sp,std::string name, T value)
{
	space = sp;
    boost::get< signal< T > >((*space)[name]).trigger(value);
}

template void resign< std::string >(space_class *sp,std::string                        name,
                                    std::function< void(std::string) > func);
template void resign< int32_t >(space_class *sp,std::string                    name,
                                std::function< void(int32_t) > func);

template void trigger< std::string >(space_class *sp,std::string name, std::string value);
template void trigger< int32_t >(space_class *sp,std::string name, int32_t value);

int lua_resign(lua_State* L)
{
    assert(lua_gettop(L) == 2);
    assert(lua_iscfunction(L, 2));
    assert(lua_isstring(L, 1));
    lua_CFunction f = lua_tocfunction(L, 2);
    size_t        size;
    const char*   name_ = lua_tolstring(L, 1, &size);
    std::string   name(name_, size);
    boost::apply_visitor([f](auto a) { a.resign(f); }, (*space)[name]);
    return 0;
}

class lua_value
{
    lua_State* L;
    int        index;

  public:
    lua_value(lua_State* L, int index);
    operator std::string();
    operator int32_t();
};

int lua_trigger(lua_State* L)
{
    assert(lua_gettop(L) == 2);
    lua_value v(L, 2);
    boost::apply_visitor([&v](auto f) { f.trigger(v); },
                         (*space)[lua_value(L, 1)]);
    return 0;
}


space_class * new_space()
{
	return new space_class();
}

void lua_init()
{
    lua_register(Ls, "resign", lua_resign);
    lua_register(Ls, "trigger", lua_trigger);
}

void lua_thread_init()
{
	Ls = lua_open();
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