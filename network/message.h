#pragma once
#include "socket.h"
namespace lsy
{
    class message_socket : public assocket
    {
        assocket* aso;
        buffer    buf;
        buffer    head;
        bool      is_head;

      public:
        message_socket(assocket* aso_);
        virtual void write(buffer buf, std::function< void() > func);
        virtual void close();

      protected:
        virtual ~message_socket();
    };
}
