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
using boost::filesystem::last_write_time;
using boost::filesystem::exists;
is_expire::is_expire(const std::string &derive)
:expire(!exists(derive) || last_write_time("H:/code/lrs/db.json")>last_write_time(derive)){}main_class::main_class()
:is_expire("main.db")
,lsy::database("main.db")
,insert(this,"INSERT INTO player VALUES (?,?);") 
,select(this,"select passwd from player where id=?;") 
{
if(expire){
auto p=new_statement("DROP TABLE main");
p->OnData.connect([p](bool flag){assert(flag);delete p;});
p->bind();
p=new_statement("CREATE TABLE main(id varchar,passwd varchar");
p->OnData.connect([p](bool flag){assert(flag);delete p;});
p->bind();
}
}
main_class main;
