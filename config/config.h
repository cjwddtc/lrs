#include <boost/config.hpp>
#include <stdint.h>
#include <string>
namespace config
{
    using namespace std::string_literals;
    constexpr uint16_t button_port = 0;
    constexpr uint16_t login_port  = 1;
    constexpr char*    room_init   = "room_init";
}