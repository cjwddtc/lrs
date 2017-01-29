/*
this file is the socket api for the lrs system
A new class inherent from the assocket and socket_getter can be complie to a dynamic link library and used by the system
*/
#pragma once
#include "tool.h"
#include <boost/config.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/signals2.hpp>
#include <thread>
namespace lsy
{
    class assocket;

    /*
	this is the socket class which is the basic class of a async socket
    */
    class BOOST_SYMBOL_EXPORT assocket:public as_mem_man
    {
      public:
      	/*
			can not be copy
      	*/
        assocket(const assocket&) = delete;
        assocket()                = default;
        /*
			this signal should be call when a message is comming to the socket
			the buffer will contain the message
        */
        boost::signals2::signal< void(const buffer) > OnMessage;
        /*
			this function will write buf to the socket an return immediately,when the write operator finish func will be called
        */
        virtual void write(buffer buf, std::function< void() > func = []() {})
            = 0;

      private:
      	/*
			this is protected mean this object can manage itself no need to delete,because the memeory will free before OnDestroy called
      	*/
        virtual ~assocket() = default;
    };

	/*
	this class is a class to async get assocket
	*/
    class BOOST_SYMBOL_EXPORT socket_getter
    {
      public:
      	/*
			this is the signal when new socekt is build
      	*/
        boost::signals2::signal< void(assocket&) > OnNewSocket;
        /*
			stop getting new socket
        */
        virtual void                               stop() = 0;
        /*
			start getting new socket on the given config and viven thread
			the format of the config is depend on the real derive class
        */
        virtual void start(boost::property_tree::ptree& config,
                           std::thread&                 thr)
            = 0;

      protected:
      	/*
			this is protected mean this object can manage itself no need to delete,because the memeory will free after stop called
      	*/
        virtual ~socket_getter() = default;
    };
}
