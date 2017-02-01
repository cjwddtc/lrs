#include "socket.h"
lsy::socket_getter::NewSocket(assocket_ptr soc)
{
    soc->bind_father(this);
    add_as(soc);
}