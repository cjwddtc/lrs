#pragma once
#include <boost/config.hpp>
#include <stdint.h>
#include <string>
namespace config
{
    using namespace std::string_literals;
	//login port
    constexpr uint16_t login_port        = 1;
	//match port
    constexpr uint16_t multiplay_port    = 2;
    constexpr uint16_t games_port        = 3;
    constexpr uint16_t match_port        = 4;
    constexpr uint16_t match_status_port = 5;

    constexpr uint16_t text_port         = 6;

    constexpr uint16_t channel_open   = 7;
	constexpr uint16_t channel_close = 12;
	constexpr uint16_t channel_enable = 10;
	constexpr uint16_t role_info = 8;
	constexpr uint16_t player_dead = 13;
	constexpr uint16_t room_info = 11;
	constexpr uint16_t public_channel = 9;
	constexpr uint16_t button_port = 0;

    constexpr char*    room_init       = "room_init";
}