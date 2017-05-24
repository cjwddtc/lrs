#pragma once
#include <port.h>
#include <stdint.h>
namespace room_space
{
	struct player
	{
		lsy::port_all *pl;
		uint8_t index;
		bool is_dead;
		lsy::port_all *operator*();
		lsy::port_all *operator->();
		uint8_t get_index();
		player(lsy::port_all *pl,uint8_t index);
		std::map<std::string, uint16_t> buttons;
		void add_button(std::string name, std::function<void(uint8_t)> func);
		void remove_button(std::string name);
	};
}