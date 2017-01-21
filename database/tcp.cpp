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
        std::atomic< int >           count;
        std::atomic< bool >          is_closing;
        tcp(boost::asio::io_service& io, size_t buf_size)
            : soc(io)
            , buf(buf_size)
            , count(0)
            , is_closing(false)
        {
        }
        virtual void read()
        {
            count++;
            soc.async_read_some(boost::asio::buffer(buf.data(), buf.size()),
                                [this](const boost::system::error_code& error,
                                       std::size_t bytes_transferred) {
                                    count--;

                                    if (error != 0)
                                    {
                                        is_closing = true;
                                    }

                                    if (is_closing && count == 0)
                                    {
                                        count = -1;
                                        soc.get_io_service().post(
                                            [this]() { delete this; });
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
        virtual void write(buffer message, std::function< void() > func)
        {
            if (is_closing)
            {
                throw closing_write();
            }

            auto buf = boost::asio::buffer(message.data(), message.size());
            count++;
            soc.async_write_some(
                buf,
                [message, func, this](const boost::system::error_code& error,
                                      std::size_t bytes_transferred) {
                    count--;
                    func();

                    if (is_closing && count == 0)
                    {
                        count = -1;
                        soc.get_io_service().post([this]() { delete this; });
                    }
                });
        }
        virtual void close()
        {
            soc.close();
            is_closing = true;
        }
        virtual ~tcp()
        {
            OnDestroy();
        }
    };

    class tcp_listener : public socket_getter
    {
        std::shared_ptr< boost::asio::io_service > io_service;
        size_t                                     buf_size;
        boost::asio::ip::tcp::acceptor*            acc;
        std::atomic< bool >                        is_closing;

      public:
        tcp_listener()
            : io_service(new boost::asio::io_service)
            , acc(0)
            , is_closing(false)
        {
        }

        void accept(tcp* ptr, const boost::system::error_code& ec)
        {
            if (is_closing)
                return;

            boost::asio::detail::throw_error(ec, "accept");
            auto c = new tcp(*io_service, buf_size);
            acc->async_accept(c->soc,
                              [c, this](const boost::system::error_code& ec) {
                                  accept(c, ec);
                              });
            OnNewSocket(*ptr);
            ptr->read();
        }

        virtual void start(boost::property_tree::ptree& config,
                           std::thread&                 thr)
        {
            buf_size = config.get("buf_size", 256);
            acc      = new boost::asio::ip::tcp::acceptor(
                *io_service,
                boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                               config.get("port", 12345)));
            auto c = new tcp(*io_service, buf_size);
            acc->async_accept(c->soc,
                              [c, this](const boost::system::error_code& ec) {
                                  accept(c, ec);
                              });
            std::thread([io = io_service]() { io->run(); }).swap(thr);
        }

        virtual void stop()
        {
            is_closing = true;
            acc->close();
            io_service->post([this]() { delete this; });
        }
        virtual ~tcp_listener()
        {
            delete acc;
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
                    OnNewSocket(*soc);
                });
            std::thread([io = io_service]() { io->run(); }).swap(thr);
        }

        virtual void stop()
        {
            io_service->post([this]() { delete this; });
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
