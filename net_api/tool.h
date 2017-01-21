#pragma once
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/config.hpp>
#include <boost/signals2.hpp>
#include <stdint.h>
namespace lsy
{
    struct count_block
    {
        size_t        count;
        unsigned char ptr[1];
    };

    class BOOST_SYMBOL_EXPORT buffer
    {
        count_block*           ptr;
        size_t                 size_;
        mutable unsigned char* now_ptr;

      public:
        buffer(size_t size_);
        buffer(const buffer& buf);

        unsigned char* begin();
        unsigned char* end();

        void*  data() const;
        void*  data();
        size_t size() const;

        void resize(size_t size);

        size_t remain() const;
        size_t readed() const;

        template < class T >
        void put(T a)
        {
            assert(false);
        }

        void put(const unsigned char* ptr, size_t size);
        void put(const buffer&);

        void get(uint16_t& t) const;
        void get(uint32_t& t) const;
        void get(unsigned char* ptr, size_t size) const;
        void get(const buffer&) const;

        template < class T >
        T get() const
        {
            assert(false);
        }

        unsigned char* get(size_t size) const;

        void reset() const;
        void renew(size_t new_size);
        void renew();

        ~buffer();
    };

    template <>
    void buffer::put< uint16_t >(uint16_t a);

    template <>
    void buffer::put< uint32_t >(uint32_t a);

    template <>
    uint16_t buffer::get< uint16_t >() const;

    template <>
    uint32_t buffer::get< uint32_t >() const;

    template < class T >
    class writer
    {
        T& value;

      public:
        writer(T& a)
            : value(a)
        {
        }
        boost::signals2::signal< void() > OnWrite;
        void send(buffer message)
        {
            value.write(message, [this]() {
                OnWrite();
                delete this;
            });
        }
    };
    template < class T >
    writer< T >& get_writer(T& value)
    {
        return *new writer< T >(value);
    }
}
