#include "socket.h"
#include <functional>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace lsy
{
    typedef websocketpp::server< websocketpp::config::asio >     server;
    typedef websocketpp::connection< websocketpp::config::asio > connect;

    class BOOST_SYMBOL_EXPORT websocket : public assocket
    {
      public:
        websocket(connect::ptr con_)
            : con(con_)
        {
            con->set_message_handler([this](auto ab, auto ptr) {
                auto&  a = ptr->get_payload();
                buffer buf(a.size());
                buf.put((const unsigned char*)a.data(), a.size());
                OnMessage(buf);
            });
        }
        connect::ptr con;
        virtual void write(buffer buf, std::function< void() > func)
        {
            con->send((const void*)buf.data(), buf.size());
            func();
        }
        virtual void close()
        {
            con->close(::websocketpp::close::status::normal, "");
        }
    };

    class websocket_listner : public socket_getter
    {
      public:
        server m_endpoint;
        websocket_listner()
        {
            m_endpoint.set_access_channels(websocketpp::log::alevel::none);
            m_endpoint.init_asio();
            m_endpoint.set_open_handler([this](auto hd) {
                websocketpp::lib::error_code ec;
                OnNewSocket(
                    *new websocket(m_endpoint.get_con_from_hdl(hd, ec)));
            });
        }

        virtual void stop()
        {
        }

        virtual void start(boost::property_tree::ptree& config,
                           std::thread&                 thr)
        {
            m_endpoint.listen(config.get("port", 9002));
            m_endpoint.start_accept();
            std::thread([this]() { m_endpoint.run(); }).swap(thr);
        }
    };

    extern "C" BOOST_SYMBOL_EXPORT socket_getter& websocket_socket_getter()
    {
        return *new websocket_listner();
    }
};
