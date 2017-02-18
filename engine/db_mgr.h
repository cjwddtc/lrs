#include "database.h"
#include <map>
namespace lsy
{
    class db_manager
    {
        class db_proxy
        {
            lsy::database db;
            std::map< std::string, lsy::database::statement > sts;

          public:
            db_proxy(db_proxy&&);
            db_proxy(const std::string& str);
            lsy::database* operator->();
            lsy::database::statement& operator[](const std::string& str);
        };

      public:
        std::map< std::string, db_proxy > dbs;
        db_manager(std::string file);
        db_proxy& operator[](const std::string& str);
    };
}