#include "message.h"
namespace lsy
{
    message_socket::message_socket(assocket& aso_)
        : aso(aso_)
        , buf(1)
        , head(4)
        , is_head(true)
    {
        aso->OnMessage.connect([this](const buffer buf_) {


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
                        buf.renew(a);
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
        bind_father(aso);
    }
    void message_socket::write(buffer buf_, std::function< void() > func)
    {
        buffer buf(4);
        buf.put((uint32_t)buf_.size());

        buf.reset();
        uint32_t a;
        buf.get(a);
        aso.write(buf);
        aso.write(buf_, func);
    }

    void message_socket::close()
    {
        aso.close();
    }
    message_socket::~message_socket()
    {
    }
}
