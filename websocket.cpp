#include "socket.h"
#include <functional>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

using namespace lsy;

buffer bf(12);
extern "C" void BOOST_SYMBOL_EXPORT asd()
{
    server m_endpoint;
    // m_endpoint.set_error_channels(websocketpp::log::elevel::all ^
    //(websocketpp::log::elevel::devel |websocketpp::log::elevel::info |));
    m_endpoint.set_access_channels(websocketpp::log::alevel::none);
    m_endpoint.init_asio();
    m_endpoint.set_open_handler(
        [&m_endpoint](websocketpp::connection_hdl nhndl) {
	    std::cout << "asd" << std::endl;
	    websocketpp::lib::error_code ec;
	    auto a = m_endpoint.get_con_from_hdl(nhndl, ec);
	    a->set_message_handler([a](auto a, auto ptr) {
	        assert(ptr->get_opcode() == websocketpp::frame::opcode::BINARY);
	        std::string str = ptr->get_raw_payload();
	        buffer bff(str.size());
	        bff.put((const unsigned char*)str.data(), str.size());
	        bff.reset();
	        std::cout << "test1:" << std::endl;
	        std::cout << bff.get<uint32_t>() << std::endl;
	        std::cout << bff.get<uint16_t>() << std::endl;
	        std::cout << bff.get<uint16_t>() << std::endl;
	        std::cout << bff.get<uint32_t>() << std::endl;
	        bf.put<uint32_t>(10);
	        bf.put<uint16_t>(1234);
	        bf.put<uint16_t>(10);
	        bf.put<uint32_t>(20);
	        a->send(bf.data(), bf.size());
	    });
	});
    m_endpoint.listen(9002);
    m_endpoint.start_accept();
    m_endpoint.run();
    return;
}
// namespace lsy{
//
// class websocket:public assocket
//{
//	server::connection_ptr p;
//	websocket(){}
//    virtual writer& write(){
//		return *new websocket_write(*this);
//	}
//	virtual void close(){
//		p->close(close::status::value::normal,"");
//	}
//};
//
// class websocket_write:public writer
//{
//	websocket &soc;
//	websocket_write(websocket &soc_):soc(soc_){}
//    virtual void send(buffer message){
//		p->send((void *)message.data(),message.size());
//		OnWrite(message.size());
//	}
//    virtual ~websocket_write()=default;
//};
//
//}