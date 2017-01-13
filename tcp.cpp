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
#include <atomic>
namespace lsy{
class tcp_acceptor;

class closing_write :public std::exception {};

class tcp :public assocket
{
public:
	boost::asio::ip::tcp::socket soc;
	buffer buf;
	std::atomic<int> count;
	std::atomic<bool> is_closing;
	tcp(boost::asio::io_service &io, size_t buf_size) :soc(io), buf(buf_size), count(0), is_closing(false){}
    virtual writer& write();
	virtual void read() {
		count++;
		soc.async_read_some(boost::asio::buffer(buf.data(), buf.size()),
			[this](const boost::system::error_code& error,
				std::size_t bytes_transferred) 
		{
			count--;
			if (error != 0)
			{
				is_closing = true;
			}
			if (is_closing && count == 0) 
			{
				count = -1;
				soc.get_io_service().post([this]() {delete this; });
			}
			else 
			{
				buffer buf_(buf);
				buf_.resize(bytes_transferred);
				buf.reset();
				OnMessage(buf_);
				read();
			}
		});
	}
	virtual void close() 
	{
		soc.close();
		is_closing = true;
	}
	virtual ~tcp(){
        OnDestroy();
    }
};

class tcp_write:public write
{
    tcp &soc;
public:
    tcp_write(tcp &soc_):soc(soc_){}
    virtual void send(buffer message);
    virtual void ~tcp_write()=default;
};


virtual write& tcp::write()
{
    return *new write(*this);
}

virtual void tcp_write::send(buffer message)
{
    if (soc.is_closing) {
        throw closing_write();
    }
    auto buf = boost::asio::buffer(message.data(), message.size());
    soc.count++;
    soc.async_write_some(buf, [message , this](const boost::system::error_code& error,
        std::size_t bytes_transferred){
        soc.count--;
        OnWrite(bytes_transferred);
        soc.get_io_service().post([this]() {delete this; });
        if (is_closing && count == 0)
        {
            count = -1;
            soc.get_io_service().post([this]() {delete &soc; });
        }
    });
}

class tcp_listener :public socket_getter
{
	std::shared_ptr<boost::asio::io_service> io_service;
	size_t buf_size;
	boost::asio::ip::tcp::acceptor acc;
public:
	tcp_acceptor():io_service(new boost::asio::io_service),
			acc(*io_service, )
	{
	}
    
    void accept(tcp *ptr,const boost::system::error_code& ec) 
	{
		if (ec != 0) 
		{
			delete this;
			return;
		}
		auto c = new tcp(*io_service, buf_size);
		acc.async_accept(c->soc, [c, this]
		(const boost::system::error_code& ec) {accept(c, ec); });
		OnConnect(ptr);
		ptr->read();
	}
    
    virtual void start(boost::property_tree::ptree& config, std::thread &thr)
    {
        buf_size=config.get("buf_size",256);
        acc.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), config.get("port",12345)));
		auto c = new tcp(*io_service, buf_size);
		acc.async_accept(c->soc, [c,this]
		(const boost::system::error_code& ec) {a->accept(c, ec); });
		std::thread([io_service]() {std::cout << "start" << std::endl; io->run(); std::cout << "finish" << std::endl; }).swap(thr);
		return a;
    }
    
	virtual void stop()
	{
		acc.close();
        io_service->post([this](){delete this;})
	}
};

extern "C" BOOST_SYMBOL_EXPORT acceptor * tcp_listen(boost::property_tree::ptree &config, std::thread &thr)
{
	return tcp_acceptor::listen(std::ref(config),std::ref(thr));
}

extern "C" BOOST_SYMBOL_EXPORT boost::signals2::signal<void(assocket *)> *tcp_connect(boost::property_tree::ptree &config, std::thread &thr)
{
	auto io=std::make_shared<boost::asio::io_service>();
	auto soc=new tcp(*io,config.get("buf_size",128));
	auto sig=new boost::signals2::signal<void(assocket *)>();
	soc->soc.async_connect(boost::asio::ip::tcp::endpoint(
				boost::asio::ip::address::from_string(config.get("ip","127.0.0.1")),
			config.get("port",12345)),
			[sig,soc](const boost::system::error_code& error){
		if(error!=0){
			delete soc;
			(*sig)(0);
			delete sig;
		}
		else{
			soc->read();
			(*sig)(soc);
			delete sig;
		}
	});
	std::thread([io](){io->run();}).swap(thr);
	return sig;
}

}