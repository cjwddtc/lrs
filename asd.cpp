#include "asd.h"
decltype(player) player={.id=db_attribute("player","id"),.passed=db_attribute("player","passed")};
std::string get_table_name(decltype("player")&){
 return "player";
}
