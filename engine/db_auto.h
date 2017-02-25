#include "database.h"
#include <boost/filesystem.hpp>
namespace db_gen{
class is_expire
{
protected:
bool expire; 
is_expire(const std::string &derive); 
};
class main_class_base
:private is_expire 
,public lsy::database 
{
public:
main_class_base();
};
class main_class
:public main_class_base
{
public:
main_class();
statement insert;
statement select;
};
extern main_class main;
}
