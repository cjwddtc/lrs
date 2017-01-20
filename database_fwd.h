#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/variant.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <sqlite3.h>
#include <thread>
#include <utility>
namespace lsy
{
    template < class T >
    class value
    {
      public:
        T    val[2];
        bool is_change;
        operator const T&();
        value& operator=(T value_);
        void set(T value_);
    };

    class const_blob
    {
      public:
        const void* ptr;
        size_t      size;
        const_blob(const void* ptr, size_t size);
    };

    class blob
    {
      public:
        void*  ptr;
        size_t size;
        blob(void* ptr, size_t size);
        operator const_blob();
    };

    class run_sql_fail_exception : public std::exception
    {
      public:
        std::string str;
        run_sql_fail_exception(std::string str_)
            : str(str_)
        {
        }
    };

    class database
    {
        sqlite3*                      db;
        std::thread                   thr;
        boost::asio::io_service       io;
        boost::asio::io_service::work work;

      public:
        database(std::string path);
        ~database();
        sqlite3*                 get_db();
        void                     stop();
        boost::asio::io_service& get_io_service();
        class statement
        {
          protected:
            sqlite3_stmt*            st;
            boost::asio::io_service& io;
            void                     reset();

          public:
            void bind_(const std::string& str, int n);
            void bind_(const_blob& m, int n);
            void bind_(int m, int n);
            void bind_(double m, int n);


            ~statement();
            statement(database& db, std::string sql);

            template < size_t n = 1, class T, class... ARG >
            void bind(T a, ARG... arg)
            {
                if (n == 1)
                {
                    reset();
                }

                bind_(a, n);
                bind< n + 1 >(arg...);
            }
            template < size_t n >
            void              bind()
            {
            }
            class proxy
            {
                sqlite3_value* value;

              public:
                proxy(sqlite3_value* value_);

                operator std::string();
                operator const_blob();
                operator int();
                operator double();
            };

            class unexpect_busy : public std::exception
            {
            };

            proxy operator[](int n);


            bool run()
            {
                switch (sqlite3_step(st))
                {
                    case SQLITE_BUSY:
                        return run();
                    case SQLITE_ROW:
                        return true;
                    case SQLITE_DONE:
                        return false;
                    default:
                        std::cout << std::string(
                                         sqlite3_errmsg(sqlite3_db_handle(st)))
                                  << std::endl;
                        // I know it is the wrong using just change later
                        throw(run_sql_fail_exception(std::string(
                            sqlite3_errmsg(sqlite3_db_handle(st)))));
                }
            }

            template < class T >
            void async_run(std::function< void(T) > func)
            {
                io.post([func, this]() {
                    run();
                    func((*this)[0]);
                });
            }
            void async_run(std::function< void() > func)
            {
                io.post([func, this]() {
                    run();
                    func();
                });
            }
        };
    };

    template < class T >
    class bind_base
    {
      public:
        database&   db;
        std::string select;
        class value_type
        {
          public:
            template < class F >
            value_type(value< F > T::*a, const std::string& name_)
                : val(a)
                , name(name_)
            {
            }
            boost::variant< value< const_blob > T::*, value< int > T::*,
                            value< double > T::*, value< std::string > T::* >
                        val;
            std::string name;
        };
        bind_base(database& db_)
            : db(db_)
        {
        }
        std::map< std::string, std::vector< value_type > > values;
        template < class F >
        void add(value< F > T::*a, const std::string& name)
        {
            auto p = name.find('.');
            assert(p != std::string::npos);
            values[name.substr(0, p)].push_back(
                value_type(a, name.substr(p + 1)));
        }
        void gen_select()
        {
            select           = "select ";
            std::string from = "from ";
            std::string tmp;
            for (auto& a : values)
            {
                for (auto& b : a.second)
                {
                    select += a.first;
                    select += '.';
                    select += b.name;
                    select += ',';
                }
                select.back() = ' ';
                from += a.first;
                from += ' ';
            }

            select += from;
        }
        class statement : public database::statement
        {
            bind_base< T >& bind_class;

          public:
            statement(bind_base< T >& bind_, std::string condition);
            void async_run_func(std::function< void(T&) > func);
            void async_run(std::function< void(T&) > func)
            {
                io.post([this, func]() { async_run_func(func); });
            }
        };
    };


    template < class T >
    bind_base< T >::statement::statement(bind_base< T >& bind_,
                                         std::string     condition)
        : database::statement(bind_.db, bind_.select + condition)
        , bind_class(bind_)

    {
    }

    template < class T >
    class is_double : public boost::static_visitor< bool >
    {
      public:
        template < class F >
        constexpr bool operator()(value< F > T::*a) const
        {
            return false;
        }
        constexpr bool operator()(value< double > T::*a) const
        {
            return true;
        }
    };

    template < class T >
    class bind_data : public boost::static_visitor<>
    {
      public:
        T&                   set;
        int*                 update_index;
        int*                 where_index;
        database::statement& st;
        bind_data(T& set_, int* update_index_, int* where_index_,
                  database::statement& st_)
            : set(set_)
            , update_index(update_index_)
            , where_index(where_index_)
            , st(st_)
        {
        }
        template < class F >
        void apply_change(value< F > T::*a) const
        {
            if ((set.*a).is_change)
            {
                st.bind_((set.*a).val[0], ++(*update_index));
            }
        }
        template < class F >
        void operator()(value< F > T::*a) const
        {
            apply_change(a);
            st.bind_((set.*a).val[1], ++(*where_index));
        }
        void operator()(value< double > T::*a) const
        {
            apply_change(a);
        }
    };

    template < class T >
    class is_change : public boost::static_visitor< bool >
    {
        T* set;

      public:
        is_change(T* set_)
            : set(set_)
        {
        }
        template < class F >
        bool operator()(value< F > T::*a) const
        {
            return (set->*a).is_change;
        }
    };

    template < class T >
    class bind_value : public boost::static_visitor<>
    {
      public:
        sqlite3_value* val;
        T*             set;
        bind_value(T* set_)
            : set(set_)
        {
        }
        void operator()(value< const_blob > T::*a) const
        {
            (set->*a).set(
                const_blob(sqlite3_value_blob(val), sqlite3_value_bytes(val)));
        }

        void operator()(value< std::string > T::*a) const
        {
            (set->*a).set(std::string((const char*)sqlite3_value_text(val)));
        }

        void operator()(value< int > T::*a) const
        {
            (set->*a).set(sqlite3_value_int(val));
        }

        void operator()(value< double > T::*a) const
        {
            (set->*a).set(sqlite3_value_double(val));
        }
    };

    template < class T >
    void
    bind_base< T >::statement::async_run_func(std::function< void(T&) > func)
    {

        if (run())
        {

            T               t_value;
            bind_value< T > b(&t_value);
            int             i = 0;

            for (auto& a : bind_class.values)
            {
                for (auto& c : a.second)
                {
                    b.val = sqlite3_column_value(st, i++);
                    boost::apply_visitor(b, c.val);
                }
            }

            func(t_value);
            is_change< T > d(&t_value);
            std::string    table_name;
            std::string    sql;

            for (auto& a : bind_class.values)
            {
                std::string str  = "UPDATE " + a.first + " SET ";
                std::string str_ = "WHERE ";
                int         i    = 0;
                for (auto& c : a.second)
                {
                    if (boost::apply_visitor(d, c.val))
                    {
                        i++;
                        str += c.name;
                        str += " = ? ";
                    }
                    if (!boost::apply_visitor(is_double< T >(), c.val))
                    {
                        str_ += c.name;
                        str_ += " = ? ";
                        str_ += "AND ";
                    }
                }
                str_.erase(str_.end() - 4, str_.end());
                str += str_;
                str += ";";
                auto dp = std::make_shared< database::statement >(bind_class.db,
                                                                  str);
                database::statement& st = *dp;
                int                  p  = 0;
                bind_data< T >       asd(t_value, &p, &i, st);
                for (auto& c : a.second)
                {
                    boost::apply_visitor(asd, c.val);
                }
                st.async_run([dp]() {});
                io.post([this, func]() { async_run_func(func); });
            }
        }
    }
}