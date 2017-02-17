#include "database.h"
#include <map>
class db_manager
{
  public:
    std::map< std::string, lsy::database > dbs;
    db_manager(std::string file);
    lsy::database& operator[](const std::string& str);
};