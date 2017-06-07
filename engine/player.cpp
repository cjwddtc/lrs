#include "player.h"
#include <config.h>
#include <db_auto.h>
#include <match.h>
#include <room.h>
using db_gen::main;
extern thread_local boost::asio::io_service io_service;

lsy::port_all* lsy::player::operator->()
{
    return ptr;
}

lsy::player::player(port_all* soc, std::string id_)
    : as_contain< port_all >(soc)
    , id(id_)
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
        main.get_extra_room.bind([games_port](bool is_fin) {
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

	lsy::as_ptr< lsy::port > muti_info_port = ptr->resign_port(config::muti_info_port);
	muti_info_port->OnMessage.connect([this](lsy::buffer buf) {
		std::string name((char*)buf.get(0));
		main.get_room_info.bind_once([name,this](bool have_data) {
			if (have_data) {
				std::string info = main.get_room_info[0];
				main.get_score.bind_once([info=std::move(info),this](bool have_data) {
					if (have_data) {
						int score = main.get_score[0];
						ptr->ports[config::muti_info_port]->write(info, []() {});
						ptr->ports[config::muti_info_port]->write(uint16_t(score), []() {});
					}
				}, id, name);
			}
		},name);
	});
	muti_info_port->start();

    lsy::as_ptr< lsy::port > match_port = ptr->resign_port(config::match_port);
    match_port->OnMessage.connect(
        [ this, match_port, &io = io_service ](buffer buf) {
            std::string str((char*)buf.data());
            if (room_space::get_playing(id) == 0)
            {
                io.post([str, this]() { add_to_queue(str, this, true); });
            }
            else
            {
                match_port->write(uint16_t(0), []() {});
            }

        });
    match_port->start();


	lsy::as_ptr< lsy::port > game_info_port = ptr->resign_port(config::game_info_port);
	game_info_port->OnMessage.connect([this](lsy::buffer buf) {
		std::string name((char*)buf.get(0));
		main.get_room_info.bind_once([name, this](bool have_data) {
			if (have_data) {
				std::string info = main.get_room_info[0];
				ptr->ports[config::game_info_port]->write(info, []() {});
			}
		}, name);
	});
	game_info_port->start();

    lsy::as_ptr< lsy::port > match_noscore_port
        = ptr->resign_port(config::match_noscore_port);
    match_noscore_port->OnMessage.connect(
        [ this, match_port, &io = io_service ](buffer buf) {
            std::string str((char*)buf.data());
            if (room_space::get_playing(id) == 0)
            {
                io.post([str, this]() { add_to_queue(str, this, false); });
            }
            else
            {
                match_port->write(uint16_t(0), []() {});
            }

        });
    match_noscore_port->start();

    ptr->resign_port(config::match_noscore_status_port)->start();
    ptr->resign_port(config::match_status_port)->start();
}