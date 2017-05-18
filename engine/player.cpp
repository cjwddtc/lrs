#include "player.h"
#include <db_auto.h>
#include <config.h>
using db_gen::main;
extern thread_local boost::asio::io_service io_service;
lsy::port_all* lsy::player::operator->()
{
    return ptr;
}

lsy::player::player(port_all* soc)
    : as_contain< port_all >(soc)
{
    lsy::as_ptr< lsy::port > multiplay_port
        = ptr->resign_port(config::multiplay_port);
    multiplay_port->OnMessage.connect([this, multiplay_port](buffer buf) {
        main.get_base_room.bind(
            [& io = io_service, multiplay_port ](bool is_fin) {
                if (is_fin)
                {
                    multiplay_port->write(lsy::buffer(size_t(1)), []() {});
                }
                else
                {
                    std::string room_name = main.get_base_room[0];
                    multiplay_port->write(room_name, []() {});
                }
            });
    });
	multiplay_port->start();
    lsy::as_ptr< lsy::port > games_port = ptr->resign_port(config::games_port);
    games_port->OnMessage.connect([this, games_port](buffer buf) {
        main.get_base_room.bind([& io = io_service, games_port ](bool is_fin) {
            if (is_fin)
            {
                games_port->write(lsy::buffer(size_t(1)), []() {});
            }
            else
            {
                std::string room_name = main.get_extra_room[0];
                games_port->write(room_name, []() {});
            }
        });
    });
	games_port->start();
}