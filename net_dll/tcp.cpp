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

    class count_close
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
            assert(count);
            if (!count)
                delete this;
        }

        void inc()
        {
            ++count;
        }
        bool dec()
        {
            assert(count);
            --count;
            if (is_enable && !count)
            {
                delete this;
                return false;
            }
            else
            {
                return true;
            }
        }

        virtual ~count_close() = default;
    };
    class BOOST_SYMBOL_EXPORT tcp : public assocket, private count_close
    {
      public:
        boost::asio::ip::tcp::socket soc;
        buffer                       buf;
#ifndef NDEBUG
        bool flag;
#endif
        tcp(boost::asio::io_service& io, size_t buf_size)
            : soc(io)
            , buf(buf_size)
        {
#ifndef NDEBUG
            flag = true;
#endif
        }
        virtual void start()
        {
#ifndef NDEBUG
            assert(flag);
            flag = false;
#endif
            inc();
            soc.async_read_some(boost::asio::buffer(buf.data(), buf.size()),
                                [this](const boost::system::error_code& error,
                                       std::size_t bytes_transferred) {
                                    if (error != 0)
                                    {
                                        OnError(error);
                                        enable();
                                    }
                                    if (dec() && error == 0)
                                    {
                                        buffer buf_(buf);
                                        buf_.resize(bytes_transferred);
                                        OnMessage(buf_);
                                        if (soc.is_open())
                                        {
#ifndef NDEBUG
                                            flag = true;
#endif
                                            start();
                                        }
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
                    if (error != 0)
                    {
                        OnError(error);
                        enable();
                    }
                    func();
                    dec();
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
        std::unique_ptr< tcp >                            ptr;

      public:
        tcp_listener()
        {
        }

        void accept(const boost::system::error_code& ec)
        {
            if (ec != 0)
            {
                OnError(ec);
            }
            else
            {
                OnNewSocket(ptr.release());
            }
            if (acc->is_open())
            {
                std::make_unique< tcp >(io_service, buf_size).swap(ptr);
                acc->async_accept(ptr->soc,
                                  [this](const boost::system::error_code& ec) {
                                      accept(ec);
                                  });
            }
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
            std::make_unique< tcp >(io_service, buf_size).swap(ptr);
            acc->async_accept(
                ptr->soc,
                [this](const boost::system::error_code& ec) { accept(ec); });
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
