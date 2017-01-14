#include "port.h"

lsy::port_all::port_all(assocket &soc_):soc(soc_),buf(0),head(6),is_head(true), ports(65536)
{
	soc.OnMessage.connect([this](buffer mes)
	{
		Message_handle(mes);
	});
	soc.OnDestroy.connect([this]()
	{
		for (int i = 0; i < 65536; i++) 
		{
			if (ports[i]) 
			{
				delete ports[i];
			}
		}
		delete this;
	});
}

void lsy::port_all::Message_handle(buffer mes) 
{
	while (mes.remain())
	{
		if (is_head)
		{
			head.put(mes);
			if (head.remain() == 0)
			{
				is_head = false;
				head.reset();
				auto size = head.get<uint32_t>();
				buf.renew(size);
			}
		}
		else {
			buf.put(mes);
			if (buf.remain() == 0)
			{
				auto po = ports[head.get<uint16_t>()];
				if (po) 
				{
					po->OnMessage(buf);
				}
				head.reset();
				is_head = true;
			}
		}
	}
}

lsy::port_write::port_write(port &soc_):soc(soc_){}

void lsy::port_write::send(buffer buf)
{
	assert(buf.size() < 0xfffffff0);
	uint32_t size = buf.size();
	buffer head(6);

	head.put(size);
	head.put(soc.num);
    auto &w=soc.all.soc.write();
    w.OnWrite.connect([this](auto a){OnWrite(a);});
    soc.all.soc.write().send(head);
    w.send(buf);
}

lsy::port::port(port_all &all_, uint16_t num_):all(all_),num(num_)
{

}


lsy::port* lsy::port_all::resign_port(uint16_t num)
{
	if(ports[num])
	{
		throw port_using(num);
	}
	ports[num]=new port(*this,num);
	return ports[num];
}

void lsy::port::close()
{
	all.ports[num]=0;
	delete this;
}

lsy::port_all::port_using::port_using(uint16_t port_):port(port_)
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

lsy::writer& lsy::port::write()
{
	return *new port_write(*this);
}
