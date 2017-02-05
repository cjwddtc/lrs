#include "port.h"
lsy::port_all::port_all(assocket* soc)
    : as_contain< assocket >(soc)
{
    ptr->OnMessage.connect([this](buffer mes) {
        buffer head(2);
        head.put(mes);
        head.reset();
        uint16_t port;
        head.get(port);
        buffer data(mes.remain());
        data.put(mes);
        if (ports[port].valid())
        {
            ports[port]->OnMessage(data);
        }
        else
        {
            std::clog << "port:" << port << " is not open,skip the message\n";
        }
    });
}

lsy::port::port(port_all* all, uint16_t num_)
    : as_contain< port_all >(all)
    , num(num_)
{
}

lsy::port* lsy::port_all::resign_port(uint16_t num)
{
    if (ports[num].valid())
    {
        throw port_using(num);
    }
    return new port(this, num);
}


lsy::assocket* lsy::port_all::get_soc()
{
    return ptr;
}

void lsy::port::close()
{
    ptr->ports[num].detach();
    delete this;
}

lsy::port_all::port_using::port_using(uint16_t port_)
    : port(port_)
{
}

void lsy::port_all::close()
{
    ptr->close();
}

void lsy::port::write(buffer buf, std::function< void() > func)
{
    ptr->write(num, buf, func);
}

void lsy::port_all::write(uint16_t port, buffer buf,
                          std::function< void() > func)
{
    buffer head(2 + buf.size());
    head.put(port);
    head.put(buf);
    ptr->write(head, func);
}

void lsy::port_all::start()
{
    ptr->start();
}

void lsy::port::start()
{
    ptr->add_map(this);
}

void lsy::port_all::add_map(port* p)
{
    ports[p->num] = p;
}
