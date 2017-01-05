#include <boost/dll.hpp>
#include <boost/property_tree/ptree.hpp>
#include <thread>
#include <iostream>
#include "listener.h"

typedef lsy::acceptor *acceptor_fun(boost::property_tree::ptree &, std::thread &);

void lsy::listener::add(std::string name,boost::property_tree::ptree &pt)
{
	std::thread thr;
	auto ptr=boost::dll::import<acceptor_fun>
				(pt.get<std::string>("lib_path"),
				pt.get("listen","listen"))
			(pt,thr);
	auto &value=accs[name];
	value.first=ptr;
	value.second.swap(thr);
	ptr->OnConnect.connect([this](assocket *p){
		OnConnect(*new port_all(*p));
	});
}

void lsy::listener::add_group(boost::property_tree::ptree &pt)
{
	for(auto pt_:pt){
		add(pt_.first,pt_.second);
	}
}
