#include <channel.h>
#include <channels.h>
using namespace room_space;
room_space::channel::channel(std::string str, room_space::player* pl,
                             uint16_t port_, channels* chs_)
    : key(str, pl)
    , port(port_)
    , chs(chs_)
{
}
const std::string& channel::name() const
{
    return key.first;
}
player* channel::player() const
{
    return key.second;
}

void room_space::channel::open() const
{
    if (!(*player())->ports[port].valid())
    {
        auto p = (*player())->resign_port(port);
        p->OnMessage.connect([this](auto buf) {
            if (is_enable)
            {
                auto& log_ = chs->log[name()];
                log_.emplace_back(player()->index,
                                  std::string((char*)buf.get(0)));
                uint8_t     n  = player()->index;
                uint32_t    si = log_.size();
                lsy::buffer buf_(buf.size() + 5);
                buf_.put(si);
                buf_.put(&n, 1);
                buf_.put(buf);
                chs->for_name_channel(name(), [buf_](auto ch) {
                    (*ch->player())->ports[ch->port]->write(buf_, []() {});
                });
            }
        });
        p->start();
        lsy::buffer buf(name().size() + 3);
        buf.put(port);
        buf.put(name());
        (*player())->ports[config::channel_open]->write(buf, []() {});
    }
    else
    {
        printf("");
    }
}

void channel::enable(bool is_enable)
{
    this->is_enable = is_enable;
    lsy::buffer buf(name().size() + 3);
    buf.put((uint16_t)is_enable);
    buf.put(name());
    printf("enable:%d\n", is_enable);
    (*player())->ports[config::channel_enable]->write(buf, []() {});
}

room_space::channel::~channel()
{
    (*player())->ports[port]->close();
}
