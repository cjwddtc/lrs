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
statement select;
statement get_rule;
statement get_role_ver;
statement get_role;
statement get_base_room;
statement get_extra_room;
};
extern main_class main;
}
namespace db_gen{
using boost::filesystem::last_write_time;
using boost::filesystem::exists;
using boost::filesystem::exists;
is_expire::is_expire(const std::string &derive)
:expire(!exists(derive) || last_write_time("I:/lrs/db.json")>last_write_time(derive)){}
main_class_base::main_class_base()
:is_expire("main.db")
,lsy::database("main.db")
{
if(expire){
{
auto p=new_statement("DROP TABLE IF EXISTS player");
p->bind([p](bool flag){assert(flag);delete p;});
p=new_statement("CREATE TABLE player(id int,name varchar,passwd varchar,is_valid bool)");
p->bind([p](bool flag){assert(flag);delete p;});
}
{
auto p=new_statement("DROP TABLE IF EXISTS friend");
p->bind([p](bool flag){assert(flag);delete p;});
p=new_statement("CREATE TABLE friend(ida int,idb int,message varchar)");
p->bind([p](bool flag){assert(flag);delete p;});
}
{
auto p=new_statement("DROP TABLE IF EXISTS rule");
p->bind([p](bool flag){assert(flag);delete p;});
p=new_statement("CREATE TABLE rule(rulename varchar,explain varchar,configfile varchar,luafile varchar,onwerid int)");
p->bind([p](bool flag){assert(flag);delete p;});
}
{
auto p=new_statement("DROP TABLE IF EXISTS role");
p->bind([p](bool flag){assert(flag);delete p;});
p=new_statement("CREATE TABLE role(rolename varchar,explain varchar,configfile varchar,luafile varchar,onwerid int)");
p->bind([p](bool flag){assert(flag);delete p;});
}
{
auto p=new_statement("DROP TABLE IF EXISTS role_ver");
p->bind([p](bool flag){assert(flag);delete p;});
p=new_statement("CREATE TABLE role_ver(rolename varchar,vername varchar,explain varchar,configfile varchar,luafile varchar,onwerid int)");
p->bind([p](bool flag){assert(flag);delete p;});
}
{
auto p=new_statement("DROP TABLE IF EXISTS role_extend");
p->bind([p](bool flag){assert(flag);delete p;});
p=new_statement("CREATE TABLE role_extend(rolefather varchar,roleson varchar)");
p->bind([p](bool flag){assert(flag);delete p;});
}
{
auto p=new_statement("DROP TABLE IF EXISTS room");
p->bind([p](bool flag){assert(flag);delete p;});
p=new_statement("CREATE TABLE room(room_name varchar,rulename varchar,explain varchar,type int)");
p->bind([p](bool flag){assert(flag);delete p;});
}
{
auto p=new_statement("DROP TABLE IF EXISTS room_contain");
p->bind([p](bool flag){assert(flag);delete p;});
p=new_statement("CREATE TABLE room_contain(room_name varchar,rolename varchar,vername varchar,count size)");
p->bind([p](bool flag){assert(flag);delete p;});
}
{
auto p=new_statement("DROP TABLE IF EXISTS rule_base_role");
p->bind([p](bool flag){assert(flag);delete p;});
p=new_statement("CREATE TABLE rule_base_role(rolename varchar,explain varchar,configfile varchar,luafile varchar,onwerid int,rulename varchar)");
p->bind([p](bool flag){assert(flag);delete p;});
}
}
}
main_class::main_class()
:main_class_base()
,select(this,"select passwd from player where id=?;") 
,get_rule(this,"select luafile from rule where rulename=?;") 
,get_role_ver(this,"select luafile from role_ver where rolename=? AND vername=?") 
,get_role(this,"select luafile from role where rolename=?") 
,get_base_room(this,"select room_name from room where type=0") 
,get_extra_room(this,"select room_name from room where type=1") 
{}
main_class main;
}
