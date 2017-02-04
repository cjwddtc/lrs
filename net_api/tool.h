/*
this file is network tool for the net_api
mainly about buffer
*/
#pragma once
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/config.hpp>
#include <boost/signals2.hpp>
#include <stdint.h>

class as_ptr;
class as_ptr;
namespace lsy
{
    struct count_block
    {
        size_t        count;
        unsigned char ptr[1];
    };

    typedef boost::signals2::signal< void() > signal;

    class BOOST_SYMBOL_EXPORT as_close
    {
      protected:
        as_close() = default;

      public:
        virtual void close() = 0;
        signal       OnDestroy;
        virtual void start() = 0;
        virtual ~as_close();
    };

    template < class T >
    class as_contain
    {
      protected:
        T* ptr;

      public:
        as_contain(T* ptr_)
            : ptr(ptr_)
        {
            ptr->OnDestroy.connect([this]() { delete this; });
        }

        T* operator->() const
        {
            return ptr;
        }
    };

    template < class T >
    class as_ptr
    {
        T*                          ptr;
        boost::signals2::connection con;

      public:
        as_ptr(T* ptr_)
            : ptr(ptr_)
            , con(ptr->OnDestroy.connect([this]() { ptr = 0; }))
        {
        }
        as_ptr()
            : ptr(nullptr)
        {
        }

        as_ptr(const as_ptr& other)
            : ptr(other.ptr)
            , con(ptr->OnDestroy.connect([this]() { ptr = 0; }))
        {
        }

        void detach()
        {
            if (ptr)
            {
                ptr = nullptr;
                con.disconnect();
            }
        }

        as_ptr< T >& operator=(const as_ptr< T >& other)
        {
            detach();
            new (this) as_ptr< T >(other);
            return std::ref(*this);
        }

        as_ptr< T >& operator=(T* other)
        {
            detach();
            new (this) as_ptr< T >(other);
            return std::ref(*this);
        }

        T& operator*() const
        {
            assert(valid());
            return std::ref(*ptr);
        }

        T* operator->() const
        {
            assert(valid());
            return ptr;
        }
        bool valid() const
        {
            return ptr == nullptr;
        }
        T* get() const
        {
            return ptr;
        }
        ~as_ptr()
        {
            detach();
        }
    };

    /*
                buffer class is a set of binary datas manage by refrence counter
    */
    class BOOST_SYMBOL_EXPORT buffer
    {
        count_block*           ptr;
        size_t                 size_;
        mutable unsigned char* now_ptr;

      public:
        /*
                create a new buffer which size is given
        */
        buffer(size_t size_);
        /*
                create a buffer which share the same data with the gived buf
                but the read_write pointer will be set to start
        */
        buffer(const buffer& buf);
        /*
                get the start of the buf
        */
        unsigned char* begin();
        /*
                get the end of the buf
        */
        unsigned char* end();
        /*
                get the raw data of the buffer
        */
        void* data() const;
        /*
                get the raw data of the buffer
        */
        void* data();
        /*
                get the size of the buffer
        */
        size_t size() const;

        /*
                change the logical size of the buffer
                **warning this function will no change the real size of the
           buffer's data so size should be smaller than the real size of the
           buffer's data
                if you want to change the real size use renew
        */
        void resize(size_t size);

        /*
                the remain byte which is not read or write
        */
        size_t remain() const;
        /*
                the remain byte which is read or write
        */
        size_t readed() const;
        /*
                template funciont which should never be called
        */
        template < class T >
        void put(T a)
        {
            assert(false);
        }
        /*
                put a data ptr point to which size is size
        */
        void put(const unsigned char* ptr, size_t size);
        /*
                put a buffer into this buffer as much as possibly
        */
        void put(const buffer&);
        /*
                get a uint16_t from the buffer which is host endian
        */
        void get(uint16_t& t) const;
        /*
                get a uint32_t from the buffer which is host endian
        */
        void get(uint32_t& t) const;
        /*
                get data to ptr which size is size
        */
        void get(unsigned char* ptr, size_t size) const;
        /*
        get data to buffer from another buffer
        */
        void get(const buffer&) const;

        /*
                template funciont which should never be called
        */
        template < class T >
        T get() const
        {
            assert(false);
        }

        /*
                get data to a pointer which size is size and return the pointer
        */
        unsigned char* get(size_t size) const;

        /*
        reset the read_write pointer to the start
        */
        void reset() const;
        /*
                alloc a new memory of new_size for this buffer
                if the old memory is only own by this buffer it will be free or
           count will be decrease
        */
        void renew(size_t new_size);
        /*
                alloc a new memory of old size for this buffer
                if the old memory is only own by this buffer it will be free or
           count will be decrease
        */
        void renew();

        ~buffer();
    };

    /*
        put uint16_t of host endian to the buffer
    */
    template <>
    void buffer::put< uint16_t >(uint16_t a);

    /*
        put uint32_t of host endian to the buffer
    */
    template <>
    void buffer::put< uint32_t >(uint32_t a);

    /*
        get uint16_t of host endian from the buffer
    */
    template <>
    uint16_t buffer::get< uint16_t >() const;

    /*
        get uint32_t of host endian from the buffer
    */
    template <>
    uint32_t buffer::get< uint32_t >() const;

    /*
        a template class used to convert the call back style socket write to
       signal style
    */
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

    /*
        get the write signal
    */
    template < class T >
    writer< T >& get_writer(T& value)
    {
        return *new writer< T >(value);
    }
}
