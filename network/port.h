#ifndef PORT_H
#define PORT_H
#include "socket.h"
#include <array>
#include <assert.h>
#include <stdint.h>
namespace lsy
{

    class port_all;
    class BOOST_SYMBOL_EXPORT port : public assocket
    {
        port_all& all;

      public:
        uint16_t num;
        port(port_all& all_, uint16_t num_);
        virtual void write(buffer buf, std::function< void() > func);
        virtual void close();

      protected:
        virtual ~port();
    };

    class BOOST_SYMBOL_EXPORT port_all
    {
        assocket& soc;

      public:
        class port_using : public std::exception
        {
          public:
            uint16_t port;
            port_using(uint16_t port);
        };

        std::array< port*, 65536 > ports;

        port_all(assocket& soc_);

        port* resign_port(uint16_t num);
        void close();
        void write(uint16_t port, buffer buf, std::function< void() > fun);
    };
}
#endif
