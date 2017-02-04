#pragma once
#include "socket.h"
namespace lsy
{
    class message_socket : public assocket, private as_contain< assocket >
    {
        buffer buf;
        buffer head;
        bool   is_head;

      public:
        message_socket(assocket* aso);
        virtual void write(buffer buf, std::function< void() > func);
        virtual void close();
        virtual void start();

      protected:
        ~message_socket() = default;
    };
}
