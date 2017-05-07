#include "database.h"
#include "db_auto.h"
#include "engine.h"
#include "listener.h"
#include "socket.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/asio/deadline_timer.hpp>
using db_gen::main;
thread_local boost::asio::io_service io_service;

lsy::room::room(std::string rule_name_, std::vector<role_info> vec , std::function<void()> func):rule_name(rule_name_), space(new_space()){
	main.get_rule.bind([this,&io= io_service](bool is_fin) {
		if (is_fin) {
			std::string filename = main.get_rule[0];
			io.post([filename, this]() {
				::space = space;
				run_lua(filename);
			});
		}
	}, rule_name_);
	for (role_info &rf : vec)
	{
		//auto &role = roles[rf.first];
		//role.first = rf.second;
		//role.second = roles.size() - 1;
		size_t s=rf.second.find_first_of('.');
		if (s != std::string::npos) {
			std::string role(rf.second.begin() + s+1,rf.second.end());
			rf.second.resize(s);
			main.get_role_ver.bind([this, &io = io_service,func](bool is_fin) {
				if (is_fin) {
					std::string filename = main.get_role_ver[0];
					io.post([filename, this,func]() {
						::space = space;
						run_lua(filename);
						count--;
						if (count == 0) {
							func();
						}
					});
				}}, rf.second, role);
		}
		main.get_role.bind([this,&io = io_service](bool is_fin) {
			if (is_fin) {
				std::string filename = main.get_role[0];
				io.post([filename, this]() {
					::space = space;
					run_lua(filename);
				});
			}}, rf.second);
	}
}
/*
void lsy::room::add_role(player &p, std::string name, std::string version)
{
	roles.push_back(role(*this,p,roles.size(),name,version));
	main.get_role.bind([this](bool is_fin) {
		if (is_fin) {
			std::string filename = main.get_role[0];
			soc_get->post([filename, this]() {
				set_lua_State(L);
				run_lua(filename);
			});
		}}, name);
	if (version != "") {
		main.get_role_ver.bind([this](bool is_fin) {
			if (is_fin) {
				std::string filename = main.get_role_ver[0];
				soc_get->post([filename, this]() {
					set_lua_State(L);
					run_lua(filename);
					count--;
					if (count == 0) {
						OnReady();
					}
				});
			}}, name, version);
	}
	//main.
}*/
lsy::player::player(port_all* soc)
    : as_contain< port_all >(soc)
{
    port* p = ptr->resign_port(0);
    p->OnMessage.connect([this, p](buffer buf) {
        std::string id((char*)buf.data());
        std::string passwd((char*)buf.data() + id.size() + 1);
        main.select.bind(
            [ passwd, p, count = std::make_shared< bool >(0) ](bool have_data) {
                if (have_data)
                {
                    buffer flag(2);
                    flag.put((uint16_t)*count);
                    p->write(flag, [p, count]() {
                        if (*count)
                        {
                            p->close();
                        }
                    });
                }
                else
                {
                    buffer flag(2);
                    std::cout << passwd << std::endl;
                    std::cout << (std::string)main.select[0] << std::endl;
                    if (passwd == (std::string)main.select[0])
                    {
                        *count = 1;
                        std::cout << "login success" << std::endl;
                    }
                }
            },
            id);
    });
    p->start();
}
using db_gen::main;
lsy::engine::engine(std::string file)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml(file, pt);
    li.add_group(pt.find("engine")->second);
    li.OnConnect.connect([this](auto a) { this->ConnectHandle(a); });
}

void lsy::engine::ConnectHandle(port_all* po)
{
    auto p = new player(po);
    po->start();
}
