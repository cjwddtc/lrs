/*
* To change this license header, choose License Headers in Project Properties.
* To change this template file, choose Tools | Templates
* and open the template in the editor.
*/

#include "socket.h"
#include <atomic>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <thread>
class message;
namespace lsy
{
    class tcp_acceptor;

    class closing_write : public std::exception
    {
    };

    class tcp : public assocket
    {
      public:
        boost::asio::ip::tcp::socket soc;
        buffer                       buf;
        tcp(boost::asio::io_service& io, size_t buf_size)
            : soc(io)
            , buf(buf_size)
        {
        }
        virtual void read()
        {
            soc.async_read_some(boost::asio::buffer(buf.data(), buf.size()),
                                [self = shared_from_this()](
                                    const boost::system::error_code& error,
                                    std::size_t bytes_transferred) {

                                    if (error != 0)
                                    {
                                        self->OnError(error);
                                        self->close();
                                    }
                                    else
                                    {
                                        buffer buf_(buf);
                                        buf_.resize(bytes_transferred);
                                        buf.reset();
                                        self->OnMessage(buf_);
                                        self->read();
                                    }
                                });
        }
        virtual void write(buffer message, std::function< void() > func)
        {
            auto buf = boost::asio::buffer(message.data(), message.size());
            soc.async_write_some(buf,
                                 [ message, func, self = shared_from_this() ](
                                     const boost::system::error_code& error,
                                     std::size_t bytes_transferred) {
                                     if (error != 0)
                                     {
                                         self->OnError(error);
                                     }
                                     func();
                                 });
        }
        virtual void close()
        {
            soc.close();
        }
        virtual ~tcp()
        {
        }
    };
    typedef std::shared_ptr< tcp > tcp_ptr;
    class tcp_listener : public socket_getter
    {
        boost::asio::io_service                            io_service;
        size_t                                             buf_size;
        std::unique_ptr< boost::asio::ip::tcp::acceptor* > acc;

      public:
        tcp_listener()
        {
        }

        void accept(tcp_ptr ptr, const boost::system::error_code& ec)
        {
            if (ec != 0)
            {
                OnError(ec);
            }
            auto c = std::make_shared< tcp >(*io_service, buf_size);
            c->bind_father(shared_from_this());
            acc->async_accept(c->soc, [
                c, self = shared_from_this()
            ](const boost::system::error_code& ec) { self->accept(c, ec); });
            NewSocket(ptr);
            ptr->read();
        }

        virtual void start(boost::property_tree::ptree& config,
                           std::thread&                 thr)
        {
            buf_size = config.get("buf_size", 256);
            acc.swap(std::make_unique< boost::asio::ip::tcp::acceptor >(
                io_service,
                boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                               config.get("port", 12345))));
            auto c = std::make_shared< tcp >(*io_service, buf_size);
            acc->async_accept(c->soc, [
                c, self = shared_from_this()
            ](const boost::system::error_code& ec) { self->accept(c, ec); });
            std::thread([self = shared_from_this()]() {
                self->io_service.run();
            }).swap(thr);
        }

        virtual void close()
        {
            acc->close();
            for_each([](assocket_ptr ptr) { ptr->close(); });
        }
        virtual ~tcp_listener()
        {
        }
    };

    class tcp_connector : public socket_getter
    {
        std::shared_ptr< boost::asio::io_service > io_service;

      public:
        tcp_connector()
            : io_service(new boost::asio::io_service)
        {
        }

        virtual void start(boost::property_tree::ptree& config,
                           std::thread&                 thr)
        {
            auto soc = new tcp(*io_service, config.get("buf_size", 128));
            soc->soc.async_connect(
                boost::asio::ip::tcp::endpoint(
                    boost::asio::ip::address::from_string(
                        config.get("ip", "127.0.0.1")),
                    config.get("port", 12345)),
                [this, soc](const boost::system::error_code& error) {
                    boost::asio::detail::throw_error(error, "start");
                    soc->read();
                    NewSocket(soc);
                });
            std::thread([io = io_service]() { io->run(); }).swap(thr);
        }

        virtual void close()
        {
            destroy();
        }
    };

    extern "C" BOOST_SYMBOL_EXPORT socket_getter& tcp_listner_socket_getter()
    {
        return *new tcp_listener();
    }

    extern "C" BOOST_SYMBOL_EXPORT socket_getter& tcp_connecter_socket_getter()
    {
        return *new tcp_connector();
    }
}
