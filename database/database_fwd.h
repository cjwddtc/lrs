#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>
#include <sqlite3.h>
#include <thread>
namespace lsy
{
    class BOOST_SYMBOL_EXPORT const_blob
    {
      public:
        const void* ptr;
        size_t      size;
        const_blob(const void* ptr, size_t size);
    };

    class BOOST_SYMBOL_EXPORT blob
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


    class unexpect_busy : public std::exception
    {
    };

    class BOOST_SYMBOL_EXPORT database
    {
        sqlite3*                      db;
        std::thread                   thr;
        boost::asio::io_service       io;
        boost::asio::io_service::work work;

      public:
        database(database&& other);
        database(std::string path);

        void stop();
        ~database();

        sqlite3*                 get_db();
        boost::asio::io_service& get_io_service();

        class statement
        {
          protected:
            sqlite3_stmt*            st;
            boost::asio::io_service& io;
            void                     reset();
            void bind_(const std::string& str, int n);
            void bind_(const_blob& m, int n);
            void bind_(int m, int n);
            void bind_(double m, int n);
            bool run();

          public:
            ~statement();
            statement(database* db, std::string sql);

            template < size_t n = 1, class T, class... ARG >
            void bind(T a, ARG... arg);
            template < size_t n = -1 >
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

            boost::signals2::signal< void(bool) > OnData;

            proxy operator[](int n);
        };
        database::statement* new_statement(std::string sql);
    };
}