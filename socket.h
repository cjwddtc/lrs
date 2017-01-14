#ifndef TCP_HPP
#define TCP_HPP
#include <boost/signals2.hpp>
#include <boost/config.hpp>
#include <boost/property_tree/ptree.hpp>
#include <thread>
#include "tool.h"
namespace lsy{
class assocket;

class BOOST_SYMBOL_EXPORT writer
{
public:
	boost::signals2::signal<void(size_t)> OnWrite;
    virtual void send(buffer message)=0;
    virtual ~writer()=default;
};

class BOOST_SYMBOL_EXPORT assocket
{
public:
	boost::signals2::signal<void(buffer)> OnMessage;
	boost::signals2::signal<void()> OnDestroy;
    virtual writer& write()=0;
	virtual void close() = 0;

protected:
	virtual ~assocket() = default;
};

class BOOST_SYMBOL_EXPORT socket_getter
{
public:
	boost::signals2::signal<void(assocket&)> OnNewSocket;
	virtual void stop() = 0;
    virtual void start(boost::property_tree::ptree& config, std::thread &thr) = 0;
protected:
	virtual ~socket_getter() = default;
};
}


#endif /* TCP_HPP */