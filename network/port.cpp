#include "port.h"
uint16_t lsy::port_all::valid_port()
{
    uint16_t n;
    do
    {
        n = rand() % 65536;
    } while (ports[n].valid());
    return n;
}
lsy::port_all::port_all(assocket* soc)
    : as_contain< assocket >(soc)
{
    ptr->OnMessage.connect([this](buffer mes) {/*
        printf("in:");
        mes.print();
        printf("\n");*/
        buffer head((size_t)2);
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
			std::lock_guard<std::mutex> mut(unpost_mut);
			if (unpost[port].size() < 100); {
				unpost[port].push_back(data); 
			}
            std::clog << "port:" << port << " is not open,skip the message\n";
        }
    });
    ptr->OnError.connect([this](auto a) { OnError(a); });
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
    auto p   = new port(this, num);
    auto con = ptr->OnError.connect([p](auto ec) { p->OnError(std::ref(ec)); });
    p->OnDestroy.connect([con]() { con.disconnect(); });
    return p;
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
    ptr->write(head, func);/*
    printf("out:");
    head.print();
    printf("\n");*/
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
	//std::lock_guard<std::mutex> mut(unpost_mut);
	auto it = unpost.find(p->num);
	if (it != unpost.end())
	{
		std::clog << "port:" << p->num << " message is post\n";
		for (auto buf : it->second)
		{
			p->OnMessage(buf);
		}
		it->second.clear();
	}
}

lsy::port_all::~port_all()
{
    OnDestroy();
    OnDestroy.disconnect_all_slots();
    printf("delete port_all:%p\n", this);
}
