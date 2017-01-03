#include "socket.h"
#include <thread>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/dll.hpp>
#include <iostream>
#include "port.h"
using namespace lsy;
int main(int n,char *argv[]) {
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml("asd.xml", pt);
	std::thread thr_ser,thr_cli;
	acceptor *p=boost::dll::import<
			acceptor *(boost::property_tree::ptree &, std::thread &)
			>(pt.get<std::string>("lib_path"),"tcp_listen")(pt,thr_ser);
	p->OnConnect.connect([p](assocket *ptr) {
		ptr->OnMessage.connect([](buffer mes){;});
		auto ptr_=new port_all(*ptr);
		auto p5=ptr_->resign_port(5);
		auto p4=ptr_->resign_port(4);
		{
			auto ptr=p5;
			std::string str("Hello");
			buffer buf(str.size());
			buf.put((const unsigned char *)str.data(), str.size());
			ptr->send(buf)->connect([](size_t size) {std::cout << "Hello writed" << std::endl; });
		}
		{
			p4->OnMessage.connect([ptr,p](buffer a)
			{
				std::string b(a.begin(), a.end()); 
				std::cout << "revice:" << b << std::endl;
				ptr->close();
				p->stop();
			});
		}
	});
	boost::dll::import<boost::signals2::signal<void(assocket *)> 
			*(boost::property_tree::ptree &, std::thread &)
			>(pt.get<std::string>("lib_path"),"tcp_connect")
			(pt,thr_cli)->connect([](assocket *ptr){
				auto ptr_=new port_all(*ptr);
				auto p5=ptr_->resign_port(5);
				p5->OnMessage.connect([ptr_](buffer a) {
					std::cout << "qwe" << std::endl;
					auto p4=ptr_->resign_port(4); 
					p4->send(a)->connect([](size_t size) {std::cout << "response" << std::endl; });
				});
				
			});
	std::cout << "asd" << std::endl;
	thr_ser.join();
	thr_cli.join();
}