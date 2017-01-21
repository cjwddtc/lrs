#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/variant.hpp>
#include <functional>
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
        run_sql_fail_exception(std::string str_);
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
            void bind(T a, ARG... arg);
            template < size_t n >
            void              bind();
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


            bool run();
            template < class T >
            void async_run(std::function< void(T) > func);
            void async_run(std::function< void() > func);
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
            value_type(value< F > T::*a, const std::string& name_);
            boost::variant< value< const_blob > T::*, value< int > T::*,
                            value< double > T::*, value< std::string > T::* >
                        val;
            std::string name;
        };
        bind_base(database& db_);
        std::map< std::string, std::vector< value_type > > values;
        template < class F >
        void add(value< F > T::*a, const std::string& name);
        void gen_select();
        class statement : public database::statement
        {
            bind_base< T >& bind_class;

          public:
            statement(bind_base< T >& bind_, std::string condition);
            void async_run_func(std::function< void(T&) > func);
            void async_run(std::function< void(T&) > func);
        };
    };
}