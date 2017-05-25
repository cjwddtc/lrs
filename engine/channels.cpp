#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index_container.hpp>
#include <channels.h>
using namespace boost::multi_index;
using namespace room_space;
namespace room_space
{
    struct channel_map
        : public multi_index_container< channel,
                                        indexed_by< hashed_unique< member< channel,
                                                                           std::
                                                                               pair< std::
                                                                                         string,
                                                                                         player* >,
                                                                           &channel::
                                                                               key > >,
                                                    hashed_non_unique< const_mem_fun< channel,
                                                                                      const std::
                                                                                          string&,
                                                                                      &channel::
                                                                                          name > >,
                                                    hashed_non_unique< const_mem_fun< channel,
                                                                                          player*,
                                                                                      &channel::
                                                                                          player > > > >
    {
    };
}

channels::channels()
    : map(new channel_map())
{
}

channels::~channels()
{
    delete map;
}

channel* channels::get_channel(player* pl, std::string name)
{
    auto& iti = map->get< 0 >();
    auto  it  = iti.find(std::make_pair(name, pl));
    if (it == iti.end())
    {
		(*pl)->mut.lock();
        uint16_t port = (*pl)->valid_port();
        auto rp = (*pl)->resign_port(port);
        rp->start();
		(*pl)->mut.unlock();
		auto it=map->emplace(name, pl, port);
		it.first->open();
        rp->OnMessage.connect([ &channel = *it.first, this ](auto buf) {
            if (channel.is_enable)
            {
                auto& index = map->get< 1 >();
                auto  its   = index.equal_range(channel.name());
                auto  it    = its.first;
				uint8_t n= channel.player()->index;
				lsy::buffer buf_(buf.size() + 1);
				buf_.put(&n, 1);
				buf_.put(buf);
                while (it != its.second)
                {
                    (*it->player())->ports[it->port]->write(buf_, []() {});
                    ++it;
                }
            }
        });
        const channel& ch = *(it.first);
        return (channel*)&ch;
    }
    const channel& ch = *it;
    return (channel*)&ch;
}

void room_space::channels::remove_channel(channel * chan)
{
	printf("remove:%p\n", chan->player());
	auto &index=map->get<0>();
	auto it = index.find(chan->key);
	if (it == index.end()) {
		printf("remove no-exist channel\n");
	}
	else
	{
		auto &po = (*chan->player());
		po->ports[config::channel_close]->write(lsy::buffer(chan->name()), []() {});
		po->ports[chan->port]->close();
		map->erase(it);
	}
}

void room_space::channels::for_player_channel(player * pl, std::function<void(const channel*)> func)
{
	auto it=map->get<2>().equal_range(pl);
	while (it.first != it.second)
	{
		func(&*it.first);
		++it.first;
	}
}

void channels::sent(std::string name, std::string mes)
{
    assert(false);
}
