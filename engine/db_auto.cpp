#include "database.h"
#include <boost/filesystem.hpp>
namespace db_gen
{
    class is_expire
    {
      protected:
        bool expire;
        is_expire(const std::string& derive);
    };
    class main_class_base : private is_expire, public lsy::database
    {
      public:
        main_class_base();
    };
    class main_class : public main_class_base
    {
      public:
        main_class();
        statement select;
        statement get_rule;
        statement get_role_ver;
        statement get_role;
        statement get_base_room;
        statement get_extra_room;
        statement get_room_size;
        statement get_score;
        statement get_room_role;
        statement get_room_rule;
        statement add_score;
        statement dec_score;
        statement get_room_info;
        statement update_passwd;
        statement add_role;
        statement add_rule;
        statement add_room;
        statement add_room_role;
        statement remove_role;
        statement remove_rule;
        statement remove_room;
        statement remove_room_role;
        statement get_user_room;
        statement get_user_room_rule;
        statement get_user_rule;
        statement get_user_role;
        statement get_user_room_role;
    };
    extern main_class main;
}
namespace db_gen
{
    using boost::filesystem::last_write_time;
    using boost::filesystem::exists;
    using boost::filesystem::exists;
    is_expire::is_expire(const std::string& derive)
        : expire(!exists(derive)
                 || last_write_time("I:/lrs/db.json") > last_write_time(derive))
    {
    }
    main_class_base::main_class_base()
        : is_expire("main.db")
        , lsy::database("main.db")
    {
        if (expire)
        {
            {
                auto p = new_statement("DROP TABLE IF EXISTS player");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
                p = new_statement(
                    "CREATE TABLE player(name varchar,passwd varchar,is_valid "
                    "bool)");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
            }
            {
                auto p = new_statement("DROP TABLE IF EXISTS friend");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
                p = new_statement(
                    "CREATE TABLE friend(ida int,idb int,message varchar)");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
            }
            {
                auto p = new_statement("DROP TABLE IF EXISTS rule");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
                p = new_statement(
                    "CREATE TABLE rule(rulename varchar,explain "
                    "varchar,configfile varchar,luafile varchar,onwerid "
                    "varchar)");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
            }
            {
                auto p = new_statement("DROP TABLE IF EXISTS role");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
                p = new_statement(
                    "CREATE TABLE role(rolename varchar,explain "
                    "varchar,configfile varchar,luafile varchar,onwerid "
                    "varchar)");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
            }
            {
                auto p = new_statement("DROP TABLE IF EXISTS role_ver");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
                p = new_statement(
                    "CREATE TABLE role_ver(rolename varchar,vername "
                    "varchar,explain varchar,configfile varchar,luafile "
                    "varchar,onwerid varchar)");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
            }
            {
                auto p = new_statement("DROP TABLE IF EXISTS role_extend");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
                p = new_statement(
                    "CREATE TABLE role_extend(rolefather varchar,roleson "
                    "varchar)");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
            }
            {
                auto p = new_statement("DROP TABLE IF EXISTS room");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
                p = new_statement(
                    "CREATE TABLE room(room_name varchar,rulename "
                    "varchar,explain varchar,onwerid varchar)");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
            }
            {
                auto p = new_statement("DROP TABLE IF EXISTS room_contain");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
                p = new_statement(
                    "CREATE TABLE room_contain(room_name varchar,rolename "
                    "varchar,count size)");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
            }
            {
                auto p = new_statement("DROP TABLE IF EXISTS rule_base_role");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
                p = new_statement(
                    "CREATE TABLE rule_base_role(rolename varchar,explain "
                    "varchar,configfile varchar,luafile varchar,onwerid "
                    "varchar,rulename varchar)");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
            }
            {
                auto p = new_statement("DROP TABLE IF EXISTS score");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
                p = new_statement(
                    "CREATE TABLE score(name varchar,room_name varchar,score "
                    "int)");
                p->bind([p](bool flag) {
                    assert(flag);
                    delete p;
                });
            }
        }
    }
    main_class::main_class()
        : main_class_base()
        , select(this, "select passwd from player where name=?;")
        , get_rule(this, "select luafile from rule where rulename=?;")
        , get_role_ver(
              this,
              "select luafile from role_ver where rolename=? AND vername=?")
        , get_role(this, "select luafile from role where rolename=?")
        , get_base_room(this,
                        "select room_name from room where onwerid is null")
        , get_extra_room(this,
                         "select room_name from room where onwerid is not null")
        , get_room_size(this,
                        "select sum(count) from room_contain where room_name=?")
        , get_score(this,
                    "select score from score where name=? AND room_name=?")
        , get_room_role(
              this, "select rolename,count from room_contain where room_name=?")
        , get_room_rule(this,
                        "select luafile from room INNER JOIN rule ON "
                        "room.rulename=rule.rulename where room_name=?")
        , add_score(
              this,
              "update score set score=score+100 where name=? AND room_name=?")
        , dec_score(this,
                    "update score set score=score-100 where name=? AND "
                    "room_name=? AND score>=100")
        , get_room_info(this, "select explain from room where room_name=?")
        , update_passwd(this,
                        "update player set passwd=? where name=? AND passwd=?")
        , add_role(this,
                   "insert into role (rolename,luafile,onwerid) VALUES(?,?,?)")
        , add_rule(this,
                   "insert into rule (rulename,luafile,onwerid) VALUES(?,?,?)")
        , add_room(this,
                   "insert into room (room_name,rulename,explain,onwerid) "
                   "VALUES(?,?,'',?)")
        , add_room_role(this,
                        "insert into room_contain (room_name,rolename,count) "
                        "VALUES(?,?,?)")
        , remove_role(this, "DELETE FROM role WHERE rolename=?")
        , remove_rule(this, "DELETE FROM rule WHERE rulename=?")
        , remove_room(this, "DELETE FROM room WHERE room_name=?")
        , remove_room_role(this, "DELETE FROM room_contain WHERE room_name=?")
        , get_user_room(this, "select room_name from room where onwerid=?")
        , get_user_room_rule(this,
                             "select rulename from room where room_name=?")
        , get_user_rule(this, "select rulename from rule where onwerid=?")
        , get_user_role(this, "select rolename from role where onwerid=?")
        , get_user_room_role(this,
                             "select rolename,count from room_contain INNER "
                             "JOIN room ON "
                             "room_contain.room_name=room.room_name where "
                             "room.room_name=? AND onwerid=?")
    {
    }
    main_class main;
}
