#ifndef TCP_HPP
#define TCP_HPP
#include "tool.h"
#include <boost/config.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/signals2.hpp>
#include <thread>
namespace lsy
{
    class assocket;

    class BOOST_SYMBOL_EXPORT assocket
    {
      public:
        assocket(const assocket&) = delete;
        assocket()                = default;
        boost::signals2::signal< void(const buffer) > OnMessage;
        boost::signals2::signal< void() >             OnDestroy;
        virtual void write(buffer buf, std::function< void() > func = []() {})
            = 0;
        virtual void close() = 0;

      protected:
        virtual ~assocket() = default;
    };

    class BOOST_SYMBOL_EXPORT socket_getter
    {
      public:
        boost::signals2::signal< void(assocket&) > OnNewSocket;
        virtual void                               stop() = 0;
        virtual void start(boost::property_tree::ptree& config,
                           std::thread&                 thr)
            = 0;

      protected:
        virtual ~socket_getter() = default;
    };
}


#endif /* TCP_HPP */
