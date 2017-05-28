#include <functional>
#include <lua_bind.h>
#include <stdint.h>
using namespace room_space;
template < size_t N, class T, class... ARG >
struct get_arg
{
    typedef typename get_arg< N - 1, ARG... >::type type;
};
template < class T, class... ARG >
struct get_arg< 0, T, ARG... >
{
    typedef T type;
};

template < size_t N, class... ARG >
using arg = typename get_arg< N, ARG... >::type;
struct bad_get : public std::exception
{
    size_t n;
    bad_get(size_t N)
        : n(N)
    {
    }
};
void lua_get(lua_State* ls, int index, int& value)
{
    if (!lua_isnumber(ls, index))
    {
        throw bad_get(index);
    }
    value = lua_tointeger(ls, index);
}

void lua_get(lua_State* ls, int index, uint8_t& value)
{
    if (!lua_isnumber(ls, index))
    {
        throw bad_get(index);
    }
    value = lua_tointeger(ls, index);
}

void lua_get(lua_State* ls, int index, uint16_t& value)
{
    if (!lua_isnumber(ls, index))
    {
        throw bad_get(index);
    }
    value = lua_tointeger(ls, index);
}

void lua_get(lua_State* ls, int index, std::string& str)
{
    if (!lua_isstring(ls, index))
    {
        throw bad_get(index);
    }
    str.assign(lua_tostring(ls, index));
}

void lua_get(lua_State* ls, int index, bool& value)
{
    if (!lua_isboolean(ls, index))
    {
        throw bad_get(index);
    }
    value = lua_toboolean(ls, index);
}

template < class T, class... ARG >
struct get_put_arg_func
{
    static std::function< void(T, ARG...) > f(lua_State* ls)
    {
        return [ls](T a, ARG... b) {
            lua_put(ls, a);
            get_put_arg_func< ARG... >::f(ls)(b...);
        };
    }
};

template < class T >
struct get_put_arg_func< T >
{
    static std::function< void(T) > f(lua_State* ls)
    {
        return [ls](T a) { lua_put(ls, a); };
    }
};


template < class... ARG >
void lua_get(lua_State* ls, int index, std::function< void(ARG...) >& func)
{
    if (!lua_isfunction(ls, index))
    {
        throw bad_get(index);
    }
    lua_pushvalue(ls, index);
    int n = luaL_ref(ls, LUA_GLOBALSINDEX);
    func  = [n, ls](ARG... arg) {
        lua_rawgeti(ls, LUA_GLOBALSINDEX, n);
        get_put_arg_func< ARG... >::f(ls)(arg...);
        lua_call(ls, sizeof...(ARG), 0);
    };
}

template < class T, class... ARG >
void lua_get(lua_State* ls, int index, std::function< T(ARG...) >& func)
{
    if (!lua_isfunction(ls, index))
    {
        throw bad_get(index);
    }
    lua_pushvalue(ls, index);
    int n = luaL_ref(ls, LUA_GLOBALSINDEX);
    func  = [n, ls](ARG... arg) {
        lua_rawgeti(ls, LUA_GLOBALSINDEX, n);
        get_put_arg_func< ARG... >::f(ls)(arg...);
        lua_call(ls, sizeof...(ARG), 1);
        T a;
        lua_get(ls, -1, a);
        return a;
    };
}


void lua_get(lua_State* ls, int index, std::function< void() >& func)
{
    if (!lua_isfunction(ls, index))
    {
        throw bad_get(index);
    }
    lua_pushvalue(ls, index);
    int n = luaL_ref(ls, LUA_GLOBALSINDEX);
    func  = [n, ls]() {
        lua_rawgeti(ls, LUA_GLOBALSINDEX, n);
        lua_call(ls, 0, 0);
    };
}

template < class T, class... ARG >
void lua_get(lua_State* ls, int index, std::function< T() >& func)
{
    if (!lua_isfunction(ls, index))
    {
        throw bad_get(index);
    }
    lua_pushvalue(ls, index);
    int n = luaL_ref(ls, LUA_GLOBALSINDEX);
    func  = [n, ls](ARG... arg) {
        lua_rawgeti(ls, LUA_GLOBALSINDEX, n);
        lua_call(ls, 0, 1);
        T a;
        lua_get(ls, -1, a);
        return a;
    };
}


template < class T >
void lua_get(lua_State* ls, int index, T*& value)
{
    if (!lua_istable(ls, index))
    {
        throw bad_get(index);
    }
    lua_rawgeti(ls, index, 0);
    value = (T*)(lua_touserdata(ls, -1));
    lua_pop(ls, 1);
}

void lua_put(lua_State* ls, int a)
{
    lua_pushinteger(ls, a);
}

void lua_put(lua_State* ls, uint8_t a)
{
    lua_pushinteger(ls, a);
}

void lua_put(lua_State* ls, uint16_t a)
{
    lua_pushinteger(ls, a);
}

void lua_put(lua_State* ls, std::string a)
{
    lua_pushlstring(ls, a.data(), a.size());
}

void lua_put(lua_State* ls, bool a)
{
    lua_pushboolean(ls, a);
}


int lua_func(lua_State* ls)
{
	auto func = (std::function< int(lua_State * ls) >*)lua_touserdata(
		ls, lua_upvalueindex(1));
	return (*func)(ls);
}

template < size_t N, class F, class... ARG, class... REARG >
F w(lua_State* ls,  std::function<F(ARG...)> func, REARG... rearg)
{
	arg< N, ARG... > value;
	lua_get(ls, N + 1, value);
	return w< N + 1 >(ls, func, rearg..., value);
}

template < size_t N, class F, class... ARG >
F w(lua_State* ls,  std::function<F(ARG...)> func, ARG... arg)
{
	return func(arg...);
}

template <class T, class ...ARG>
void lua_put(lua_State* ls, std::function<T(ARG...)> func)
{
	auto qwe = new std::function< int(lua_State*) >([func](lua_State* ls) {
		try
		{
			F f = w< 0 >(ls, func);
			lua_put(ls, f);
		}
		catch (bad_get e)
		{

			printf("function %d type %s is wrong\n", e.n, lua_typename(ls, e.n));
			return 0;
		}
		return 1;
	});
	lua_pushlightuserdata(ls, (void*)qwe);
	lua_pushcclosure(ls, lua_func, 1);
}

template < class ...ARG>
void lua_put(lua_State* ls, std::function<void(ARG...)> func)
{
	auto qwe = new std::function< int(lua_State*) >([func](lua_State* ls) {
		try
		{
			w< 0 >(ls, func);
		}
		catch (bad_get e)
		{

			printf("function %d type %s is wrong\n", e.n, lua_typename(ls, e.n));
			return 0;
		}
		return 0;
	});
	lua_pushlightuserdata(ls, (void*)qwe);
	lua_pushcclosure(ls, lua_func, 1);
}

template < size_t N, class T, class F, class... ARG, class... REARG >
F q(lua_State* ls, T* ptr, F (T::*func)(ARG...), REARG... rearg)
{
    arg< N, ARG... > value;
    lua_get(ls, N + 1, value);
    return q< N + 1 >(ls, ptr, func, rearg..., value);
}

template < size_t N, class T, class F, class... ARG >
F q(lua_State* ls, T* ptr, F (T::*func)(ARG...), ARG... arg)
{
    return (ptr->*func)(arg...);
}

template < class T, class F, class... ARG >
void f(lua_State* ls, T* ptr, F (T::*func)(ARG...), std::string name)
{
    auto qwe = new std::function< int(lua_State*) >([func, ptr, name](lua_State* ls) {
        try
        {
            F f = q< 0 >(ls, ptr, func);
            lua_put(ls, f);
        }
        catch (bad_get e)
        {

            printf("function %s:%d type %s is wrong\n", name.c_str(), e.n, lua_typename(ls, e.n));
            return 0;
        }
        return 1;
    });
    lua_pushlightuserdata(ls, (void*)qwe);
    lua_pushcclosure(ls, lua_func, 1);
    lua_setfield(ls, -2, name.c_str());
}

template < class T, class... ARG >
void f(lua_State* ls, T* ptr, void (T::*func)(ARG...), std::string name)
{
    auto qwe = new std::function< int(lua_State*) >([name,func, ptr](lua_State* ls) {
        try
        {
            q< 0 >(ls, ptr, func);
        }
        catch (bad_get e)
        {
			printf("function %s:%d type %s is wrong\n", name.c_str(), e.n, lua_typename(ls, e.n));
            return 0;
        }
        return 0;
    });
    lua_pushlightuserdata(ls, (void*)qwe);
    lua_pushcclosure(ls, lua_func, 1);
    lua_setfield(ls, -2, name.c_str());
}

template < class T, class F, class... ARG, class... REMAIN >
void f_(lua_State* ls, T* ptr, std::string name, F (T::*func)(ARG...),
        REMAIN... re)
{
    f(ls, ptr, func, name);
    f_(ls, ptr, re...);
}

template < class T >
void f_(lua_State* ls, T* ptr)
{
}

template < class T, class... ARG >
void f__(lua_State* ls, T* ptr, ARG... arg)
{
    lua_newtable(ls);
    lua_pushlightuserdata(ls, (void*)ptr);
    lua_rawseti(ls, -2, 0);
    f_(ls, ptr, arg...);
}

struct A
{
    int a_;
    int f(int a)
    {
        a_ = a;
        return a;
    }
    void print()
    {
        printf("A:%d\n", a_);
    }
};

struct B
{
    uint16_t b_;
    A* f(uint16_t ch)
    {
        b_       = ch;
        auto ptr = new A;
        ptr->a_  = b_;
        return ptr;
    }
    void print()
    {
        printf("B:%d\n", b_);
    }
};
struct C
{
    B* f(A* p)
    {
        auto ptr = new B;
        ptr->b_  = p->a_;
        return ptr;
    }
    void do_asd(int a, std::function< void() > func)
    {
        printf("do:%d\n", a);
        func();
    }
};
void lua_put(lua_State* ls, A* a)
{
    f__(ls, a, "f", &A::f, "print", &A::print);
}
void lua_put(lua_State* ls, B* b)
{
    f__(ls, b, "f", &B::f, "print", &B::print);
}

void lua_put(lua_State* ls, C* c)
{
    f__(ls, c, "f", &C::f, "qwe", &C::do_asd);
}
void test()
{
    lua_State* ls = luaL_newstate();
    luaL_openlibs(ls);
    auto a = new A;
    lua_put(ls, a);
    lua_setglobal(ls, "a");
    auto b = new B;
    lua_put(ls, b);
    lua_setglobal(ls, "b");
    auto c = new C;
    lua_put(ls, c);
    lua_setglobal(ls, "c");
    printf("%d\n", luaL_dofile(ls, "asd.lua"));
    printf("%s\n", lua_tostring(ls, -1));
}
namespace room_space
{
    void setglobal(lua_State* ls, std::string name)
    {
        lua_setglobal(ls, name.c_str());
    }

    void lua_put(lua_State* ls, signals* ptr)
    {
        f__(ls, ptr, "get_signal", &signals::get_signal);
    }
    void lua_put(lua_State* ls, signal* ptr)
    {
        f__(ls, ptr, "connect", &signal::connect, "trigger", &signal::trigger);
    }
    void lua_put(lua_State* ls, channel* ptr)
    {
        f__(ls, ptr, "enable", &channel::enable);
    }
    void lua_put(lua_State* ls, channels* ptr)
    {
        f__(ls, ptr, "get_channel", &channels::get_channel, "sent",
            &channels::sent, "remove_channel", &channels::remove_channel);
    }
    void lua_put(lua_State* ls, player* ptr)
    {
        f__(ls, ptr, "add_button", &player::add_button, "remove_button",
            &player::remove_button, "sent_public", &player::sent_public,
            "set_camp", &player::set_camp,"index",&player::get_index,"is_dead",&player::dead);
    }
    void lua_put(lua_State* ls, room* ptr)
    {
        f__(ls, ptr, "signals", &room::signals, "channels", &room::channels,
            "wait", &room::wait, "set_dead", &room::is_dead, "sent_public",
            &room::sent_public, "close", &room::close, "get_player",
            &room::get_player, "size", &room::size,"for_each_player",&room::for_each_player,"load_lua",&room::load_file,"check",&room::check);
    }

	std::function<void(player*)> room::load_file(std::string filename)
	{
		if (luaL_dofile(ls, filename.c_str()))
		{
			printf("run\"%s\"wrong:%s\n", filename.c_str(),
				lua_tostring(ls, -1));
			return [](player*) {};
		}
		else
		{
			std::function<void(player *pl)> func;
			lua_get(ls, -1, func);
			return func;
		}
	}
}