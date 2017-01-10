#ifndef TCP_HPP
#define TCP_HPP
#include <boost/signals2.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/config.hpp>
namespace lsy{
struct count_block
{
	size_t count;
	unsigned char ptr[1];
};
class BOOST_SYMBOL_EXPORT buffer
{
	count_block *ptr;
	size_t size_;
	mutable unsigned char *now_ptr;
public:
	buffer(size_t size_);
	buffer(const buffer &buf);
	
	unsigned char *begin();
	unsigned char *end();

	void *data() const;
	void *data();

	size_t size() const;
	
	void resize(size_t size);

	size_t remain() const;
	size_t readed() const;

	
	template <class T>
	void put(T a)
	{
		assert(false);
	}

	template <>
	void buffer::put<uint16_t>(uint16_t a);

	template <>
	void buffer::put<uint32_t>(uint32_t a);

	void put(const unsigned char *ptr, size_t size);

	void put(const buffer &);


	void get(uint16_t &t) const;
	void get(uint32_t &t) const;
	void get(unsigned char *ptr,size_t size) const;
	void get(const buffer &) const;
	unsigned char *get(size_t size) const;


	template <class T>
	T get() const
	{
		assert(false);
		return T();
	}

	template <>
	uint16_t buffer::get<uint16_t>() const;

	template <>
	uint32_t buffer::get<uint32_t>() const;


	void reset() const;
	void renew(size_t new_size);
	void renew();

	~buffer();
};

class BOOST_SYMBOL_EXPORT assocket
{
public:
	boost::signals2::signal<void(buffer)> OnMessage;
	boost::signals2::signal<void()> OnDestroy;
	virtual boost::signals2::signal<void(size_t)> *send(
		buffer message) = 0;
	virtual void close() = 0;

protected:
	virtual ~assocket() = default;
};

class BOOST_SYMBOL_EXPORT acceptor
{
public:
	boost::signals2::signal<void(assocket *)> OnConnect;
	virtual void stop() = 0;

protected:
	virtual ~acceptor() = default;
};
}


#endif /* TCP_HPP */