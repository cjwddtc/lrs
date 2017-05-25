#pragma once
#include <port.h>
#include <stdint.h>
namespace room_space
{
	class room;
	struct player
	{
		lsy::port_all *pl;
		uint8_t camp;
		room *ro;
		uint8_t index;
		bool is_dead;
		std::string id;
		lsy::port_all *operator*();
		lsy::port_all *operator->();
		uint8_t get_index();
		player(room *ro,lsy::port_all *pl,uint8_t index, std::string id);
		std::map<std::string, uint16_t> buttons;
		void add_button(std::string name, std::function<void(uint8_t)> func);
		void remove_button(std::string name);
		void sent_public(std::string mes);
		void rebind(lsy::port_all *pl);
		bool is_null();
		void set_camp(uint8_t camp);
		~player();
		player(const player&) ;
	};
	extern std::map<std::string, player*> playing_map;
	void add_playing(std::string str, player *pl);
	player *get_playing(std::string str);
}