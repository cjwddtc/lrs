#pragma once
#include <string>
#include <db_tools.h>
namespace db_class{
struct player{
value<std::string> id;
value<std::string> passed;
};
}

extern struct {
db_attribute id;
db_attribute passed;
}player;
std::string get_table_name(decltype("player")&);

