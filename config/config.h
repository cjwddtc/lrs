#include <boost/config.hpp>
#include <stdint.h>
#include <string>
namespace config
{
    using namespace std::string_literals;
    constexpr uint16_t button_port       = 0;
    constexpr uint16_t login_port        = 1;
    constexpr uint16_t multiplay_port    = 2;
    constexpr uint16_t games_port        = 3;
    constexpr uint16_t match_port        = 4;
    constexpr uint16_t match_status_port = 5;
	constexpr uint16_t text_port = 6;
	constexpr uint16_t channel_control = 7;

	constexpr uint16_t channel_open = 0;
	constexpr uint16_t channel_enable = 1;
	constexpr uint16_t channel_disable = 2;
    constexpr char*    room_init         = "room_init";
}