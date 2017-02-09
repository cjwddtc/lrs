///@file socket.h
/// this file is the socket api for the lrs system
/// A new class inherent from the assocket and socket_getter can be complie to a
/// dynamic link library and used by the system
#pragma once
#include "tool.h"
#include <boost/config.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/signals2.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
#include <thread>
namespace lsy
{
    /// Error signal
    /// this signal should be call when a error occur
    ///@param ec the error code
    typedef boost::signals2::signal< void(const boost::system::error_code& ec) >
        error_signal;
    /// OnMessage singal
    /// this signal should be call when a message is comming to the socket
    /// @param buf the buffer contain the data
    typedef boost::signals2::signal< void(const buffer buf) > message_signal;

    /// this is the socket class which is the basic class of a async socket
    class BOOST_SYMBOL_EXPORT assocket : public as_close
    {
      public:
        /// Copy Constructor delete
        assocket(const assocket&) = delete;
        /// default Constructor
        assocket() = default;

        message_signal OnMessage;
        error_signal   OnError;
        /// start run assocket
        virtual void start() = 0;
        /// write function
        /// this function will write buf to the socket an return
        /// immediately,when the write operator finish func will be called
        ///@param buf the buffer to write
        ///@param func the func async called
        virtual void write(buffer buf, std::function< void() > func = []() {})
            = 0;

        /// Destructor
        virtual ~assocket() = default;
    };
    typedef as_ptr< assocket > assocket_ptr;
    /// NewSocket singal
    /// this signal should be call when a new socket is established
    /// @param aso the new socket
    typedef boost::signals2::signal< void(assocket* aso) > new_signal;


    /// this class is a class to async get assocket
    class BOOST_SYMBOL_EXPORT socket_getter : public as_close
    {
      public:
        new_signal   OnNewSocket;
        error_signal OnError;
        /// start getting new socket on the given config and given thread
        ///@remarks the format of the config is depend on the real derive class
        ///@param config the config save in boost::property_tree
        ///@param thr the thread service run
        virtual void start(boost::property_tree::ptree& config,
                           std::thread&                 thr)
            = 0;

      protected:
        /// Destructor
        virtual ~socket_getter() = default;
    };

    class writer : private as_contain< assocket >
    {
      public:
        /// Construtor
        ///@param a the assocket
        writer(assocket* a)
            : as_contain< assocket >(a)
        {
        }
        boost::signals2::signal< void() > OnWrite;
        void send(buffer message)
        {
            ptr->write(message, [this]() {
                OnWrite();
                delete this;
            });
        }

      private:
        virtual ~writer() = default;
    };
}
