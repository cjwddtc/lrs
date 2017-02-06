#include <db_tools.h>
#include <string>
namespace db_class
{
    struct player
    {
        value< std::string > id;
        value< std::string > passed;
    };
}

struct
{
    db_attribute id;
    db_attribute passed;
} player = {.id     = db_attribute("player", "id"),
            .passed = db_attribute("player", "passed")};
std::string get_table_name(decltype("player") &)
{
    return "player";
}
