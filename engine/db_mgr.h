#include "database.h"
#include <map>
namespace lsy
{
    namespace tmp
    {
        class database
            : public lsy::database,
              public std::map< std::string, lsy::database::statement >
        {
          public:
            database(database&& other);
            database(const std::string& str);
        };
    }
    class db_manager : public std::map< std::string, tmp::database >
    {
      public:
        db_manager(std::string file);
        tmp::database& operator[](const std::string&);
    };
}