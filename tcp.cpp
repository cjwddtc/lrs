/*
* To change this license header, choose License Headers in Project Properties.
* To change this template file, choose Tools | Templates
* and open the template in the editor.
*/

#include "socket.h"
#include <boost/dll/alias.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <thread>
namespace lsy{
class tcp_acceptor;

class tcp :public assocket
{
	tcp_acceptor *acc;
public:
	boost::asio::ip::tcp::socket soc;
	buffer buf;
	int count;
	bool is_closing;
	class closing_write :public std::exception {};
	tcp(boost::asio::io_service &io, size_t buf_size, tcp_acceptor *acc_) :soc(io), buf(buf_size), count(0), is_closing(false), acc(acc_){}
	virtual boost::signals2::signal<void(size_t)> *send(
		buffer &&message) {
		if (is_closing) {
			throw closing_write();
		}
		auto sig = new boost::signals2::signal<void(size_t)>;
		auto buf = boost::asio::buffer(message.data(), message.size());
		count++;
		soc.async_write_some(buf, [message , sig, this](const boost::system::error_code& error,
			std::size_t bytes_transferred){
			count--;
			(*sig)(bytes_transferred);
			delete sig;
			if (is_closing && count == 0)
				delete this;
		});
		return sig;
	}
	void read() {
		count++;
		soc.async_read_some(boost::asio::buffer(buf.data(), buf.size()),
			[this](const boost::system::error_code& error,
				std::size_t bytes_transferred) {
			count--;
			if (error != 0)
				is_closing = true;
			if (is_closing && count == 0)
				delete this;
			else {
				buf.resize(bytes_transferred);
				read();
				OnMessage(buf);
			}
		});
	}
	virtual void close() {
		soc.close();
		is_closing = true;
	}
	virtual ~tcp();
};

class tcp_acceptor :public acceptor
{
	boost::asio::io_service io_service;
	size_t buf_size;
	boost::asio::ip::tcp::acceptor acc;
public:
	size_t count;
	tcp_acceptor(size_t buf_size_, unsigned short port) :buf_size(buf_size_),
			acc(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)){}

	void accept(tcp *ptr,const boost::system::error_code& ec) {
		if (ec != 0) {
			if(count==0)
				delete this;
			return;
		}
		count++;
		auto c = new tcp(io_service, buf_size,this);
		acc.async_accept(c->soc, [c, this]
		(const boost::system::error_code& ec) {accept(c, ec); });
		OnConnect(ptr);
		ptr->read();
	}
	virtual void stop(){
		acc.close();
	}
	static acceptor *listen(boost::property_tree::ptree &config, std::thread &thr) {
		tcp_acceptor *a = new tcp_acceptor(config.get("buf_size",128), config.get("port",12345));
		auto c = new tcp(a->io_service, a->buf_size,a);
		a->acc.async_accept(c->soc, [c,a]
		(const boost::system::error_code& ec) {a->accept(c, ec); });
		std::thread([a]() {std::cout << "start" << std::endl; a->io_service.run(); std::cout << "finish" << std::endl; }).swap(thr);
		std::cout << "run" << std::endl;
		return a;
	}
};

tcp::~tcp() {
	OnDestroy();
	acc--;
	if (acc->count == 0)
		delete acc;
}

extern "C" BOOST_SYMBOL_EXPORT acceptor * tcp_listen(boost::property_tree::ptree &config, std::thread &thr){
	return tcp_acceptor::listen(std::ref(config),std::ref(thr));
}

}