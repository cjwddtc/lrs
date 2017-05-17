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
    };
    extern main_class main;
}
