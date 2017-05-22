#include "lua_engine.h"
#include <boost/variant.hpp>
#include <functional>
#include <map>
#include <vector>
#define get_space(s) (*(space_class*)s)
namespace lua
{

	template <size_t N, class T, class ...ARG>
	struct get_arg
	{
		typedef  typename get_arg<N - 1, ARG...>::type type;
	};	
	template <class T,class ...ARG>
	struct get_arg<0, T,ARG...>
	{
		typedef  T type;
	};

	template <size_t N, class ...ARG>
	using arg = typename get_arg<N, ARG...>::type;

	void lua_get(lua_State *ls, int index,int &value)
	{
		value = lua_tointeger(ls, index);
	}

	void lua_get(lua_State *ls, int index, uint16_t &value)
	{
		value= lua_tointeger(ls, index);
	}

	void lua_get(lua_State *ls, int index, std::string &str)
	{
		str.assign(lua_tostring(ls, index));
	}

	void lua_get(lua_State *ls, int index, bool &value)
	{
		value= lua_toboolean(ls, index);
	}

	void lua_get(lua_State *ls, int index, std::function<void()> &func)
	{
		lua_pushvalue(ls,index);
		int n = luaL_ref(ls, LUA_GLOBALSINDEX);
		func= [n, ls]() {
			lua_rawgeti(ls, LUA_GLOBALSINDEX, n);
			lua_call(ls, 0, 0);
		};
	}

	template <class T>
	void lua_get(lua_State *ls, int index,T *&value)
	{
		lua_rawgeti(ls, index, 0);
		value= (T*)(lua_touserdata(ls, -1));
		lua_pop(ls,1);
	}

	void lua_put(lua_State *ls, int a)
	{
		lua_pushinteger(ls, a);
	}

	void lua_put(lua_State *ls, uint16_t a)
	{
		lua_pushinteger(ls, a);
	}

	void lua_put(lua_State *ls, std::string a)
	{
		lua_pushlstring(ls, a.data(),a.size());
	}

	void lua_put(lua_State *ls, bool a)
	{
		lua_pushboolean(ls, a);
	}

	template <size_t N, class T, class F, class ...ARG,class... REARG>
	F q(lua_State *ls, T *ptr, F(T::*func)(ARG...), REARG ...rearg)
	{
		arg<N, ARG...> value;
		lua_get(ls, N + 1, value);
		return q<N + 1>(ls, ptr, func, rearg..., value);
	}

	template <size_t N, class T, class F, class ...ARG>
	F q(lua_State *ls,T *ptr, F(T::*func)(ARG...), ARG... arg)
	{
		return (ptr->*func)(arg...);
	}

	template <size_t N, class T,  class ...ARG, class... REARG>
	void q(lua_State *ls, T *ptr, void(T::*func)(ARG...), REARG ...rearg)
	{
		arg<N, ARG...> value;
		lua_get(ls, N + 1, value);
		q<N + 1>(ls, ptr, func, rearg..., value);
	}

	template <size_t N, class T,  class ...ARG>
	void q(lua_State *ls, T *ptr, void(T::*func)(ARG...), ARG... arg)
	{
		(ptr->*func)(arg...);
	}


	int lua_func(lua_State *ls)
	{
		auto func = (std::function<int(lua_State *ls)>*)  lua_touserdata(ls, lua_upvalueindex(1));
		return (*func)(ls);
	}
	

	template <class T, class F, class ...ARG>
	void f(lua_State *ls,T *ptr, F(T::*func)(ARG...),std::string name)
	{
		auto qwe = new std::function<int(lua_State *)>([func, ptr](lua_State *ls) {
			F f=q<0>(ls ,ptr, func);
			lua_put(ls, f);
			return 1;
		});
		lua_pushlightuserdata(ls, (void *)qwe);
		lua_pushcclosure(ls, lua_func, 1);
		lua_setfield(ls, -2, name.c_str());
	}

	template <class T, class ...ARG>
	void f(lua_State *ls, T *ptr, void(T::*func)(ARG...), std::string name)
	{
		auto qwe = new std::function<int(lua_State *)>([func, ptr](lua_State *ls) {
			q<0>(ls, ptr, func);
			return 0;
		});
		lua_pushlightuserdata(ls, (void *)qwe);
		lua_pushcclosure(ls, lua_func, 1);
		lua_setfield(ls, -2, name.c_str());
	}

	template <class T, class F, class ...ARG, class ...REMAIN>
	void f_(lua_State *ls,T *ptr,std::string name, F(T::*func)(ARG...), REMAIN ...re)
	{
		f(ls,ptr, func,name);
		f_(ls,ptr, re...);
	}

	template <class T, class F, class ...ARG>
	void f_(lua_State *ls,T *ptr, std::string name, F(T::*func)(ARG...))
	{
		f(ls,ptr, func,name);
	}

	template <class T, class ...ARG>
	void f__(lua_State *ls, T *ptr, ARG ...arg)
	{
		lua_newtable(ls);
		lua_pushlightuserdata(ls, (void*)ptr);
		lua_rawseti(ls, -2, 0);
		f_(ls, ptr, arg...);
	}

	struct A
	{
		int a_;
		int f(int a) { a_ = a; return a; }
		void print()
		{
			printf("A:%d\n", a_);
		}
	};

	struct B
	{
		uint16_t b_;
		A *f(uint16_t ch) { b_ = ch; auto ptr = new A; ptr->a_ = b_;
		return ptr;
		}
		void print()
		{
			printf("B:%d\n", b_);
		}
	};
	struct C
	{
		B *f(A *p) { auto ptr = new B; ptr->b_ = p->a_; return ptr; }
		void do_asd(int a, std::function<void()> func) { printf("do:%d\n", a); func(); }
	};
	void lua_put(lua_State *ls, A *a)
	{
		f__(ls, a,"f" ,&A::f,"print",&A::print);
	}
	void lua_put(lua_State *ls, B *b)
	{
		f__(ls, b, "f", &B::f,"print",&B::print);
	}

	void lua_put(lua_State *ls, C *c)
	{
		f__(ls, c, "f", &C::f,"qwe",&C::do_asd);
	}
	void test()
	{
		lua_State *ls = luaL_newstate();
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
		printf("%d\n",luaL_dofile(ls, "asd.lua"));
		printf("%s\n", lua_tostring(ls, -1));
	}

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


	int lua_test(lua_State* L)
	{
		assert(lua_gettop(L) == 1);
		lua_getmetatable(L, 1);
		lua_getfield(L, -1, "__index");
		printf("%s\n",lua_typename(L, -1));
		lua_pop(L, 1);
		return 1;
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

	std::vector<std::pair<std::string, lua_CFunction>> reg_funcs = {std::make_pair("resign",lua_resign),std::make_pair("trigger",lua_trigger),std::make_pair("test",lua_test) };

    void* new_space()
    {
        return (void*)new space_class();
    }

	lua_State*get_state(void * space)
	{
		return get_space(space).Ls;
	}

	void add_data(void * ptr, std::string name, std::function<void(lua_State *Ls)> func)
	{
		auto L = get_space(ptr).Ls;
		lua_getglobal(L, "global_map");
		func(L);
		lua_setfield(L, -2, name.c_str());
		lua_pop(L, 1);
	}

	void add_func(std::string name, lua_CFunction func)
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


    void run_lua(std::string file,void *space)
    {
        luaL_dofile(get_space(space).Ls, file.c_str());
    }
	int lua_table_index(lua_State * Ls)
	{
		assert(lua_gettop(Ls) == 2);
		lua_getfield(Ls, lua_upvalueindex(1),lua_tostring(Ls,2));
		return 1;
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