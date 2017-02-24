#include "database.h"
#include <boost/filesystem.hpp>
class is_expire
{
protected:
bool expire; 
is_expire(const std::string &derive); 
};
class main_class
:private is_expire 
,public lsy::database 
{
public:
main_class();
statement insert;
statement select;
};
extern main_class main;
