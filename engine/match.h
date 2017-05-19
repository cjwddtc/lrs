#include <map>
#include <player.h>
#include <string>
namespace lsy
{
    class group : public std::vector< player* >
    {
      public:
        void push_back(player* ptr);
        void pop_back();
    };
    class queue
    {
        std::map< uint16_t, group > groups;
        std::map< player*, std::pair< group*, size_t > > map;
        uint16_t    gap;
        std::string room_name;

      public:
		  size_t      size;
        void add_player(player* ptr, uint16_t score);
        void remove_player(player* ptr);
        queue(std::string room_name, size_t size, uint16_t gap);
    };
    void add_to_queue(std::string str, player* ptr);
    void remove_from_queue(std::string str, player* ptr);
}