#include "socket.h"
#include <assert.h>
#include <boost/asio/detail/socket_ops.hpp>
#include <stdint.h>
namespace lsy{
class package {
	assocket *soc;
	buffer buf;
public:
	std::vector<boost::signals2::signal<void(const std::vector<unsigned char>)>> OnMessages;

	boost::signals2::signal<void(size_t)> *write(unsigned short port, std::vector<unsigned char> vec) 
	{
		assert(vec.size() < 0xfffffff0);
		uint32_t size = vec.size()+6;
		buffer buf(size);

		buf.put(boost::asio::detail::socket_ops::host_to_network_long(size));
		buf.put(boost::asio::detail::socket_ops::host_to_network_short(port));
		buf.put(vec.data(), vec.size());

		soc->send(std::move(buf));
	}

	void Message_handle(const buffer &buf) 
	{
		uint32_t size;
		buf.get(size);
	}
	package(assocket *soc_):soc(soc_),buf(10){
		
	}

};
}

int main() {

}