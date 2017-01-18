#include "port.h"

lsy::port_all::port_all(assocket& soc_)
    : soc(soc_)
{
    soc.OnMessage.connect([this](buffer mes) {
        buffer head(2);
        head.put(mes);
        head.reset();
        uint16_t port;
        head.get(port);
        buffer data(mes.remain());
        data.put(mes);
        ports[port]->OnMessage(data);
    });
    soc.OnDestroy.connect([this]() {
        for (int i = 0; i < 65536; i++)
        {
            if (ports[i])
            {
                ports[i]->close();
            }
        }

        delete this;
    });
}

lsy::port::port(port_all& all_, uint16_t num_)
    : all(all_)
    , num(num_)
{
}

lsy::port* lsy::port_all::resign_port(uint16_t num)
{
    if (ports[num])
    {
        throw port_using(num);
    }

    ports[num] = new port(*this, num);
    return ports[num];
}

void lsy::port::close()
{
    all.ports[num] = 0;
    delete this;
}

lsy::port_all::port_using::port_using(uint16_t port_)
    : port(port_)
{
}

lsy::port::~port()
{
    OnDestroy();
}

void lsy::port_all::close()
{
    soc.close();
}

void lsy::port::write(buffer buf, std::function< void() > func)
{
    all.write(num, buf, func);
}

void lsy::port_all::write(uint16_t port, buffer buf,
                          std::function< void() > func)
{
    buffer head(2 + buf.size());
    head.put(port);
    head.put(buf);
    soc.write(head, func);
}
