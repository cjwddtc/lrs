#include "socket.h"
#include <thread>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/dll.hpp>
#include <iostream>
#include "port.h"
#include "database.h"
#include "listener.h"
using namespace lsy;

typedef acceptor *acceptor_fun(boost::property_tree::ptree &, std::thread &);
#include <sqlite3.h>
#include <boost/mpl/vector.hpp>

int main(int n,char *argv[]) {
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml("asd.xml", pt);
	listener li;
	li.add_group(pt.find("net_dll")->second);
	li.OnConnect.connect([&li](port_all &p){
		auto ptr4=p.resign_port(5);
		auto ptr5=p.resign_port(4);
		std::string str("Hello");
		buffer buf(str.size());
		buf.put((const unsigned char *)str.data(), str.size());
		ptr4->send(buf)->connect([](size_t size) {std::cout << "Hello writed" << std::endl; });
		ptr5->OnMessage.connect([ptr4,&p,&li](buffer a)
		{
			std::string b(a.begin(), a.end()); 
			std::cout << "revice:" << b << std::endl;
			p.close();
			li.close();
		});
	});
	std::thread thr_cli;
	boost::dll::import<boost::signals2::signal<void(assocket *)> 
			*(boost::property_tree::ptree &, std::thread &)
			>(pt.get<std::string>("lib_path"), pt.get("connect","connect"))
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
	thr_cli.join();
	li.join();
}