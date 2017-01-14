#include <boost/property_tree/ptree.hpp>
#include <thread>
#include <iostream>
#include "listener.h"
#include <boost/dll/import.hpp>

typedef lsy::socket_getter *acceptor_fun();

void lsy::listener::add(std::string name,boost::property_tree::ptree &pt)
{
	std::thread thr;
	auto ptr=boost::dll::import<acceptor_fun>
				(pt.get<std::string>("lib_path"),
				pt.get("listen","listen"))
			();
	auto &value=accs[name];
	value.first=ptr;
	ptr->OnNewSocket.connect([this](assocket &p){
		OnConnect(*new port_all(p));
	});
	ptr->start(pt,value.second);
}

void lsy::listener::add_group(boost::property_tree::ptree &pt)
{
	for(auto pt_:pt){
		add(pt_.first,pt_.second);
	}
}

lsy::listener::listener()
{
}

void lsy::listener::join()
{
	for(auto &a:accs)
	{
		a.second.second.join();
	}
}


void lsy::listener::close()
{
	for(auto &a:accs)
	{
		a.second.first->stop();
	}
}