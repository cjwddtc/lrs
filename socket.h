#ifndef TCP_HPP
#define TCP_HPP
#include <boost/signals2.hpp>
#include <vector>
#include <memory>
class assocket
{
public:
	boost::signals2::signal<void(const std::vector<char> &)> OnMessage;
	virtual boost::signals2::signal<void(size_t)> *send(
		std::vector<char> &&message) = 0;
	virtual void close() = 0;
protected:
	virtual ~assocket() = default;
};

class acceptor
{
public:
	boost::signals2::signal<void(assocket *)> OnConnect;
	virtual void stop()=0;
	virtual ~acceptor() = default;
};


#endif /* TCP_HPP */