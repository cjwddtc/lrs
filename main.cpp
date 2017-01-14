#include "socket.h"
#include <thread>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/dll.hpp>
#include <iostream>
#include "port.h"
#include "listener.h"
using namespace lsy;

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
		auto &w=ptr4->write();
		w.OnWrite.connect([](size_t size) {std::cout << "Hello writed" << std::endl; });
		w.send(buf);
		ptr5->OnMessage.connect([ptr4,&p,&li](buffer a)
		{
			std::string b(a.begin(), a.end()); 
			std::cout << "revice:" << b << std::endl;
			p.close();
			li.close();
		});
	});
	listener lii;
	lii.add_group(pt.find("client")->second);
	std::thread thr_cli;
	lii.OnConnect.connect([&lii](port_all &p){
		auto p5=p.resign_port(5);
		p5->OnMessage.connect([&p](buffer a) {
			std::cout << "qwe" << std::endl;
			auto p4=p.resign_port(4); 
			auto &w=p4->write();
			w.OnWrite.connect([](size_t size) {std::cout << "response" << std::endl; });
			w.send(a);
		});
	});
	lii.join();
	li.join();
}