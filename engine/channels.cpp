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

channels& room_space::channels::operator=(const channels&)
{
    assert(false);
    return *this;
}

channel* channels::get_channel(player* pl, std::string name)
{
    auto& iti = map->get< 0 >();
    auto  it  = iti.find(std::make_pair(name, pl));
    if (it == iti.end())
    {
        (*pl)->mut.lock();
        uint16_t port = (*pl)->valid_port();
        auto     it   = map->emplace(name, pl, port, this);
        it.first->open();
        (*pl)->mut.unlock();
        const channel& ch = *(it.first);
        return (channel*)&ch;
    }
    const channel& ch = *it;
    return (channel*)&ch;
}

void room_space::channels::remove_channel(channel* chan)
{
    auto& index = map->get< 0 >();
    auto  it    = index.find(chan->key);
    if (it == index.end())
    {
        printf("remove no-exist channel\n");
    }
    else
    {
        auto& po = (*chan->player());
        po->ports[config::channel_close]->write(lsy::buffer(chan->name()),
                                                []() {});
        po->ports[chan->port]->close();
        map->erase(it);
    }
}

void room_space::channels::for_player_channel(
    player* pl, std::function< void(const channel*) > func)
{
    auto it = map->get< 2 >().equal_range(pl);
    while (it.first != it.second)
    {
        func(&*it.first);
        ++it.first;
    }
}

void room_space::channels::for_name_channel(
    std::string name, std::function< void(const channel*) > func)
{
    auto it = map->get< 1 >().equal_range(name);
    while (it.first != it.second)
    {
        func(&*it.first);
        ++it.first;
    }
}

void channels::sent(std::string name, std::string mes)
{
    log[name].emplace_back(0xff, mes);
    auto        it = map->get< 1 >().equal_range(name);
    uint8_t     n  = 0xff;
    uint32_t    si = log[name].size();
    lsy::buffer buf_(mes.size() + 6);
    buf_.put(si);
    buf_.put(&n, 1);
    buf_.put(mes);
    while (it.first != it.second)
    {
        (*it.first->player())->ports[it.first->port]->write(buf_, []() {});
        ++it.first;
    }
}

void room_space::channels::resent(const channel* ch)
{
    uint32_t si = 0;
    for (auto a : log[ch->name()])
    {
        lsy::buffer buf_(a.second.size() + 6);
        buf_.put(++si);
        buf_.put(&a.first, 1);
        buf_.put(a.second);
        (*ch->player())->ports[ch->port]->write(buf_, []() {});
    }
}
