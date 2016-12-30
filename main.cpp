#include "socket.h"
#include <thread>
#include<boost/property_tree/xml_parser.hpp>
#include <boost/dll.hpp>
#include <iostream>

int main(int n,char *argv[]) {
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml("asd.xml", pt);
	std::thread thr;
	acceptor *p=boost::dll::import<
			acceptor *(boost::property_tree::ptree &config, std::thread &thr)
			>("libtcp.so","tcp_listen")(pt,thr);
	p->OnConnect.connect([p](assocket *ptr) {
		std::cout << "new soc" << ptr << std::endl;
		std::string str("Hello");
		std::vector<char> vec(str.begin(), str.end());
		ptr->send(std::move(vec))->connect([](size_t size) {std::cout << "Hello writed" << std::endl; });
		ptr->OnMessage.connect(boost::signals2::at_back,[ptr](const std::vector<char> &a) {if (std::find(a.begin(), a.end(), 'q') != a.end()) ptr->close(); });
		ptr->OnMessage.connect(boost::signals2::at_back,[p, ptr](const std::vector<char> &a) {if (std::find(a.begin(), a.end(), 'w') != a.end()) { p->stop(); ptr->close(); } });
		ptr->OnMessage.connect([](const std::vector<char> &a) {std::string b(a.begin(), a.end()); std::cout << b << std::endl; });
	});
	std::cout << "asd" << std::endl;
	getchar();
	thr.join();
}