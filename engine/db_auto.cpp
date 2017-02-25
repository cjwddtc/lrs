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
namespace db_gen{
using boost::filesystem::last_write_time;
using boost::filesystem::exists;
using boost::filesystem::exists;
is_expire::is_expire(const std::string &derive)
:expire(!exists(derive) || last_write_time("/home/lsy/lrs/db.json")>last_write_time(derive)){}
main_class_base::main_class_base()
:is_expire("main.db")
,lsy::database("main.db")
{
if(expire){
auto p=new_statement("DROP TABLE IF EXISTS player");
p->OnData.connect([p](bool flag){assert(flag);delete p;});
p->bind();
p=new_statement("CREATE TABLE player(id varchar,passwd varchar)");
p->OnData.connect([p](bool flag){assert(flag);delete p;});
p->bind();
}
}
main_class::main_class()
:main_class_base()
,insert(this,"INSERT INTO player VALUES (?,?);") 
,select(this,"select passwd from player where id=?;") 
{}
main_class main;
}
