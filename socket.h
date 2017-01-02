#ifndef TCP_HPP
#define TCP_HPP
#include <boost/signals2.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <vector>
#include <memory>
namespace lsy{
class buffer
{
	mutable unsigned char *ptr;
	size_t size_;
	mutable unsigned char *now_ptr;
public:
	buffer(size_t size_);
	buffer(const buffer &buf);

	void *data() const;
	void *data();

	size_t size() const;

	void resize(size_t);

	void realloc(size_t);

	size_t remain() const;

	template <class T>
	void put(T a){}
	
	void put(unsigned char *ptr, size_t size);

	void get(uint16_t &t) const ;
	void get(uint32_t &t) const;

	template <class T>
	T get() const
	{
		assert(false);
	}

	unsigned char *get(size_t size) const;

	~buffer();
};

template <>
void buffer::put<uint16_t>(uint16_t a)
{
	*(uint16_t *)now_ptr = 
			boost::asio::detail::socket_ops::host_to_network_short(a);
	now_ptr += 2;
}

template <>
void buffer::put<uint32_t>(uint32_t a)
{
	*(uint32_t *)now_ptr = 
			boost::asio::detail::socket_ops::host_to_network_short(a);
	now_ptr += 4;
}


template <>
uint16_t buffer::get<uint16_t>() const
{
	uint16_t t = *(uint16_t *)now_ptr;
	now_ptr += 2;
	return boost::asio::detail::socket_ops::network_to_host_short(t);
}


template <>
uint32_t buffer::get<uint32_t>() const
{
	uint32_t t = *(uint32_t *)now_ptr;
	now_ptr += 4;
	return boost::asio::detail::socket_ops::network_to_host_long(t);
}


class assocket
{
public:
	boost::signals2::signal<void(const buffer &)> OnMessage;
	boost::signals2::signal<void()> OnDestroy;
	virtual boost::signals2::signal<void(size_t)> *send(
		buffer &&message) = 0;
	virtual void close() = 0;

protected:
	virtual ~assocket() = default;
};

class acceptor
{
public:
	boost::signals2::signal<void(assocket *)> OnConnect;
	virtual void stop() = 0;

protected:
	virtual ~acceptor() = default;
};
}


#endif /* TCP_HPP */