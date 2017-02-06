#include "message.h"
namespace lsy
{
    message_socket::message_socket(assocket* aso)
        : as_contain< assocket >(aso)
        , buf(1)
        , head(4)
        , is_head(true)
    {
        ptr->OnMessage.connect([this](const buffer buf_) {
            while (buf_.remain())
            {
                if (is_head)
                {
                    head.put(buf_);

                    if (head.remain() == 0)
                    {
                        head.reset();
                        uint32_t a;
                        head.get(a);
                        if (a == 0)
                        {
                            a++;
                        }
                        buf.renew(a);
                        head.reset();
                        is_head = false;
                    }
                }
                else
                {
                    buf.put(buf_);

                    if (buf.remain() == 0)
                    {
                        OnMessage(buf);
                        is_head = true;
                    }
                }
            }
        });
        ptr->OnError.connect([this](auto a) { OnError(std::ref(a); });
    }
    void message_socket::write(buffer buf_, std::function< void() > func)
    {
        buffer buf(4);
        buf.put((uint32_t)buf_.size());
        ptr->write(buf);
        ptr->write(buf_, func);
    }

    void message_socket::close()
    {
        ptr->close();
    }

    void message_socket::start()
    {
        ptr->start();
    }
}
