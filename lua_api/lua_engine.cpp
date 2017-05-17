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

    template < class T >
    class signal
    {
        typedef boost::variant< std::string, std::function< void(T) > >
            function;
        class function_run : public boost::static_visitor<>
        {
            T value;

          public:
            function_run(T value_)
                : value(value_)
            {
            }
            void operator()(std::string func) const
            {
                space->get_table();
                lua_getfield(Ls, lua_gettop(Ls), func.c_str());
                assert(lua_isfunction(Ls, lua_gettop(Ls)));
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

    class space_class
        : public std::map< std::string, boost::variant< signal< std::string >,
                                                        signal< int32_t > > >
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

    template < class T >
    void resign(std::string name, std::function< void(T) > func, void* sp)
    {
        if (sp == nullptr)
        {
            sp = (void*)space;
        }
        boost::get< signal< T > >((*(space_class*)sp)[name]).resign(func);
    }
    template < class T >
    void trigger(std::string name, T value, void* sp)
    {
        if (sp != nullptr)
        {
            set_context(sp);
        }
        printf("%s\n", (*space)[name].type().name());
        boost::get< signal< T > >((*space)[name]).trigger(value);
    }


    template < class T >
    void declare(std::string str, void* sp)
    {
        if (sp == nullptr)
        {
            sp = (void*)space;
        }
        ((space_class*)sp)
            ->insert(
                std::pair< std::string, boost::variant< signal< std::string >,
                                                        signal< int32_t > > >(
                    str, signal< T >()));
    }

    template BOOST_SYMBOL_EXPORT void
    resign< std::string >(std::string                        name,
                          std::function< void(std::string) > func, void* sp);
    template BOOST_SYMBOL_EXPORT void
    resign< int32_t >(std::string name, std::function< void(int32_t) > func,
                      void* sp);

    template BOOST_SYMBOL_EXPORT void
    trigger< std::string >(std::string name, std::string value, void* sp);

    template BOOST_SYMBOL_EXPORT void
    trigger< int32_t >(std::string name, int32_t value, void* sp);
    template BOOST_SYMBOL_EXPORT void declare< std::string >(std::string,
                                                             void* sp);
    template BOOST_SYMBOL_EXPORT void declare< int32_t >(std::string, void* sp);

    int lua_resign(lua_State* L)
    {
        assert(lua_gettop(L) == 2);
        assert(lua_isstring(L, 2));
        assert(lua_isstring(L, 1));
        size_t      size;
        const char* name_ = lua_tolstring(L, 1, &size);
        std::string name(name_, size);
        const char* func_ = lua_tolstring(L, 2, &size);
        std::string func(func_, size);
        boost::apply_visitor([func](auto& a) { a.resign(func); },
                             (*space)[name]);
        return 0;
    }

    int lua_trigger(lua_State* L)
    {
        assert(lua_gettop(L) == 2);
        lua_value v(L, 2);
        boost::apply_visitor([&v](auto& f) { f.trigger(v); },
                             (*space)[lua_value(L, 1)]);
        return 0;
    }

    int lua_declare(lua_State* L)
    {
        assert(lua_gettop(L) == 2);
        assert(lua_isstring(L, 1));
        assert(lua_isnumber(L, 2));
        std::string str(lua_tostring(L, 1));
        switch (lua_tointeger(L, 2))
        {
            case 1:
                space->insert(std::pair< std::string,
                                         boost::variant< signal< std::string >,
                                                         signal< int32_t > > >(
                    str, signal< int32_t >()));
                break;
            case 2:
                space->insert(std::pair< std::string,
                                         boost::variant< signal< std::string >,
                                                         signal< int32_t > > >(
                    str, signal< std::string >()));
                break;
            default:
                break;
        }
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
        lua_register(Ls, "declare", lua_declare);
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
}