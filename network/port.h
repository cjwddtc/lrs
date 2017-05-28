#ifndef PORT_H
#define PORT_H
#include "socket.h"
#include <array>
#include <assert.h>
#include <mutex>
#include <stdint.h>
namespace lsy
{

    class port_all;
    class BOOST_SYMBOL_EXPORT port : public assocket,
                                     protected as_contain< port_all >
    {

      public:
        uint16_t num;
        port(port_all* all, uint16_t num_);
        virtual void write(buffer buf, std::function< void() > func);
        virtual void close();
        virtual void start();

      protected:
        virtual ~port() = default;
    };

    class BOOST_SYMBOL_EXPORT port_all : public as_close,
                                         private as_contain< assocket >
    {

      public:
        class port_using : public std::exception
        {
          public:
            uint16_t port;
            port_using(uint16_t port);
        };
        std::array< as_ptr< port >, 65536 > ports;
        std::mutex mut;
        std::mutex unpost_mut;
        std::map< uint16_t, std::vector< buffer > > unpost;
        uint16_t valid_port();
        port_all(assocket* soc);
        error_signal  OnError;
        virtual port* resign_port(uint16_t num);
        void add_map(port* p);
        virtual void close();
        virtual void start();
        assocket*    get_soc();
        void write(uint16_t port, buffer buf, std::function< void() > fun);
        virtual ~port_all();
    };
}
#endif
