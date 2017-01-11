#pragma once
#include <sqlite3.h>
#include <boost/variant.hpp>
#include <map>
#include <utility>
#include <thread>
#include <boost/asio/io_service.hpp>
#include <functional>

template <class T>
class proxy
{
	T value[2];
	operator T () {
		return value;
	}
	proxy &operator=(T value_) {
		value = value_;
	}
};

class const_blob
{
public:
	const void *ptr;
	size_t size;
	const_blob(const void *ptr, size_t size);
};

class blob
{
public:
	void *ptr;
	size_t size;
	blob(void *ptr, size_t size);
	operator const_blob();
};

class run_sql_fail_exception : public std::exception {
public:
	std::string str;
	run_sql_fail_exception(std::string str_) : str(str_) {}
};

class database
{
	sqlite3 *db;
	std::thread thr;
	boost::asio::io_service io;
	boost::asio::io_service::work work;
public:
	database(std::string path);
	~database();
	boost::asio::io_service &get_io_service();
	class statement
	{
		sqlite3_stmt *st;
		boost::asio::io_service &io;
		void bind_(std::string str, int n);
		void bind_(blob m, int n);
		void bind_(int m, int n);
		void bind_(float m, int n);

		void reset();

	public:
		~statement();
		statement(database *db, std::string sql);

		template <size_t n = 1, class T, class... ARG>
		void bind(T a, ARG... arg)
		{
			if (n == 1)
			{
				reset();
			}
			bind_(a, n);
			bind<n + 1>(arg...);
		}
		void bind();

		class proxy {
			sqlite3_value *value;

		public:
			proxy(sqlite3_value *value_);

			operator const char *();
			operator const_blob ();
			operator int();
		};

		class unexpect_busy:public std::exception
		{
		};

		proxy operator[](int n);

		template <class T>
		void async_run(std::function<void(T)> func) 
		{
			io.post([func,this]() 
			{
				switch (sqlite3_step(st))
				{
				case SQLITE_BUSY:
					throw unexpect_busy();
					break;
				case SQLITE_ROW:
					func((*this)[0]);
					break;
				default:
					//I know it is the wrong using just change later
					throw(run_sql_fail_exception(
						std::string(sqlite3_errmsg(sqlite3_db_handle(st)))));
				}
			});
		}
		void async_run(std::function<void(void)> func);
	};
};

template <class T>
class bind_base
{
public:
    database *db;
	std::string select;
	class value_type
	{
	public:
		template <class F>
		value_type(proxy<F> T::*a, const std::string &name_) :
			value(a), name(name_) {}
		boost::variant<proxy<int> T::*, proxy<int> T::*, proxy<std::string> T::*> value;
		std::string name;
	};
	bind_base(database *db_) :db(db_) {}
	std::multimap<std::string, value_type> values;
	template <class F>
	void add(proxy<F> T::*a, const std::string &name)
	{
		auto p = name.find('.');
		assert(p != std::string::npos);
		values.insert(std::make_pair(name.substr(0, p), value_type(a, name.substr(p + 1))));
	}
	void gen_select()
	{
		select = "select ";
		std::string from;
		std::string tmp;
		for (auto &a : values)
		{
			select += a.first;
			select += '.';
			select += a.second.name;
			select += ' ';
			if (tmp != a.first)
			{
				tmp = a.first;
				from += tmp;
				from += ' ';
			}
		}
		select += from;
	}
	class statement :public database::statement
	{
		bind_base<T> &bind;
		statement(bind_base<T> &bind,std::string condition);
		void async_run(std::function<void(T&)> func);
	};
};
