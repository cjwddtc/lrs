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

    class BOOST_SYMBOL_EXPORT count_close
    {
        std::atomic< unsigned > count;
        bool                    is_enable;

      public:
        count_close()
            : count(0)
            , is_enable(false)
        {
        }

        void enable()
        {
            is_enable = true;
            if (!count)
                delete this;
        }

        void inc()
        {
            ++count;
        }
        void dec()
        {
            assert(count);
            --count;
            if (is_enable && !count)
                delete this;
        }

        virtual ~count_close() = default;
    };
    class BOOST_SYMBOL_EXPORT tcp : public assocket, private count_close
    {
      public:
        boost::asio::ip::tcp::socket soc;
        buffer                       buf;

        tcp(boost::asio::io_service& io, size_t buf_size)
            : soc(io)
            , buf(buf_size)
        {
        }
        virtual void start()
        {
            inc();
            soc.async_read_some(boost::asio::buffer(buf.data(), buf.size()),
                                [this](const boost::system::error_code& error,
                                       std::size_t bytes_transferred) {
                                    dec();
                                    if (error != 0)
                                    {
                                        OnError(error);
                                        enable();
                                    }
                                    else
                                    {
                                        buffer buf_(buf);
                                        buf_.resize(bytes_transferred);
                                        buf.renew();
                                        start();
                                        OnMessage(buf_);
                                    }
                                });
        }
        virtual void write(buffer message, std::function< void() > func)
        {
            auto buf = boost::asio::buffer(message.data(), message.size());
            inc();
            soc.async_write_some(
                buf,
                [message, func, this](const boost::system::error_code& error,
                                      std::size_t bytes_transferred) {
                    dec();
                    if (error != 0)
                    {
                        OnError(error);
                        enable();
                    }
                    func();
                });
        }
        virtual void close()
        {
            soc.close();
            enable();
        }
        virtual ~tcp()
        {
        }
    };

    typedef tcp*              tcp_ptr;
    class BOOST_SYMBOL_EXPORT tcp_listener : public socket_getter
    {
        boost::asio::io_service                           io_service;
        size_t                                            buf_size;
        std::unique_ptr< boost::asio::ip::tcp::acceptor > acc;

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
            auto c = new tcp(io_service, buf_size);
            acc->async_accept(c->soc,
                              [c, this](const boost::system::error_code& ec) {
                                  accept(c, ec);
                              });
            OnNewSocket(ptr);
        }

        virtual void start(boost::property_tree::ptree& config,
                           std::thread&                 thr)
        {
            buf_size = config.get("buf_size", 256);
            std::make_unique< boost::asio::ip::tcp::acceptor >(
                io_service,
                boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                               config.get("port", 12345)))
                .swap(acc);
            auto c = new tcp(io_service, buf_size);
            acc->async_accept(c->soc,
                              [c, this](const boost::system::error_code& ec) {
                                  accept(c, ec);
                              });
            std::thread([this]() {
                io_service.run();
                delete this;
            }).swap(thr);
        }

        virtual void close()
        {
            acc->close();
        }
    };
    class tcp_connector : public socket_getter
    {
        boost::asio::io_service io_service;
        tcp*                    soc;

      public:
        tcp_connector()
        {
        }

        virtual void start(boost::property_tree::ptree& config,
                           std::thread&                 thr)
        {
            soc = new tcp(io_service, config.get("buf_size", 128));
            soc->soc.async_connect(
                boost::asio::ip::tcp::endpoint(
                    boost::asio::ip::address::from_string(
                        config.get("ip", "127.0.0.1")),
                    config.get("port", 12345)),
                [this](const boost::system::error_code& error) {
                    boost::asio::detail::throw_error(error, "start");
                    soc->start();
                    OnNewSocket(soc);
                });
            std::thread([this]() {
                io_service.run();
                delete this;
            }).swap(thr);
        }

        virtual void close()
        {
            soc->close();
        }
    };

    extern "C" BOOST_SYMBOL_EXPORT socket_getter* tcp_listner_socket_getter()
    {
        return new tcp_listener();
    }

    extern "C" BOOST_SYMBOL_EXPORT socket_getter* tcp_connecter_socket_getter()
    {
        return new tcp_connector();
    }
}
