#include "database.h"
#include <map>
namespace lsy
{
    template < class T >
    class uncreate_map : public T
    {
      public:
        typename T::mapped_type& operator[](const typename T::key_type& key)
        {
            auto it = T::find(key);
            assert(it != T::end());
            return it->second;
        }
    };
    namespace tmp
    {
        class database
            : public lsy::database,
              public uncreate_map< std::map< std::string,
                                             lsy::database::statement > >
        {
          public:
            database(database&& other);
            database(const std::string& str);
        };
    }
    class db_manager
        : public uncreate_map< std::map< std::string, tmp::database > >
    {
      public:
        db_manager(std::string file);
    };
}