#pragma once
#include "listener.h"
#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>
#include <tuple>
#include <utility>
#include <room.h>
extern thread_local boost::asio::io_service io_service;

namespace lsy
{
	class BOOST_SYMBOL_EXPORT run_thread
	{
		std::unique_ptr<boost::asio::io_service::work> work;
		boost::thread thr;
	public:
		void run();
		void stop();
		boost::asio::io_service &get_io_service();
		void add_room(std::string rule_name_, std::vector< role_info > vec);
	};
	class BOOST_SYMBOL_EXPORT server
	{
	public:
		listener li;
		std::vector<run_thread> threads;
		server(std::string file);
		void create_room(std::string rule_name_, std::vector< role_info > vec);
	};
	BOOST_SYMBOL_EXPORT void run();
}
