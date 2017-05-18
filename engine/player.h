#include <string>
#include <listener.h>
namespace lsy
{
    class player : private as_contain< port_all >
    {
      protected:
        std::string id;

      public:
        player(port_all* soc);
        port_all* operator->();
    };

}