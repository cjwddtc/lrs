/*
this file is network tool for the net_api
mainly about buffer
*/
#pragma once
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/config.hpp>
#include <boost/signals2.hpp>
#include <memory>
#include <set>
#include <shared_mutex>
#include <mutex>
#include <stdint.h>

namespace lsy
{
    struct count_block
    {
        size_t        count;
        unsigned char ptr[1];
    };

    typedef boost::signals2::signal< void() >       signal;
    typedef boost::signals2::signal< void(size_t) > error_signal;

    class BOOST_SYMBOL_EXPORT as_close
        : public std::enable_shared_from_this< as_close >
    {
      protected:
        as_close() = default;

      public:
        virtual void close() = 0;
        signal       OnDestroy;
        virtual ~as_close();
    };
    typedef std::shared_ptr< as_close > as_close_ptr;

    template < class T >
    class son_close : public T
    {
      protected:
        as_close_ptr                father;
        boost::signals2::connection con;
        void bind_father(as_close_ptr father_)
        {
            father.swap(father_);
            con = father->OnDestroy.connect([self = T::share_from_this()]() {
                self->close();
            });
        }
        virtual ~son_close()
        {
            con.disconnect();
        }
    };

    template < class T >
    class as_gather
    {
        std::shared_mutex              mut;
        std::set< std::weak_ptr< T > > as_closes;
        void add_as(std::shared_ptr< T > ptr)
        {
            mut.lock();
            as_closes.insert(ptr);
            mut.unlock();
            ptr->OnDestroy.connect([ wptr = (std::weak_ptr< T >)ptr, this ]() {
                mut.lock();
                as_closes.remove(wptr);
                mut.unlock();
            });
        }
        void for_each(std::function< void(std::shared_ptr< T >) > func)
        {
            mut.lock_shared();
            for (auto a : as_closes)
            {
                func(a.lock());
            }
            unlock_shared();
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
