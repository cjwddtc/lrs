/*! \file tool.h
this file is network tool for the net_api
mainly about buffer and close_api
*/
#pragma once
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/config.hpp>
#include <boost/signals2.hpp>
#include <iostream>
#include <stdint.h>

namespace lsy
{
    /*!use for buffer's refrence count*/
    struct count_block
    {
        /// refrence count
        size_t count;
        /// the end of the struct
        unsigned char ptr[1];
    };
    /// the no return no param signal
    typedef boost::signals2::signal< void() > signal;
    /*!the base class of async close
    the async network class cannot be close immediately
    this class define some virtual function for the async close module*/
    class BOOST_SYMBOL_EXPORT as_close
    {
      protected:
        as_close() = default;

      public:
        /// async class the class
        virtual void close() = 0;
        /// the class destroy's signal
        signal OnDestroy;

      protected:
        /// async close should delete itself so destructer is protected
        virtual ~as_close();
    };
    /*! Helper Classes for the async close module
    private inherent from this class and give the contain async class as the
    template param to this template class
    then pass the contain async class to the construct function,this class will
    delete this when contain async class is destroyed*/
    template < class T >
    class as_contain
    {
      protected:
        /// pointer to the contain async class
        T* ptr;
        /// Constructor Construct from the pointer to the contain async class
        ///@param pointer to the contain async class
        as_contain(T* ptr_)
            : ptr(ptr_)
        {
            ptr->OnDestroy.connect([this]() { delete this; });
        }
    };
    /*!async close ptr:safe pointer to the async close class
    it will be nullptr when the async close object is closed*/
    template < class T >
    class as_ptr
    {
        T*                          ptr;
        boost::signals2::connection con;

      public:
        /// Constructor Construct from the pointer to the contain async class
        ///@param pointer to the contain async class
        as_ptr(T* ptr_)
            : ptr(ptr_)
            , con(ptr->OnDestroy.connect([this]() { ptr = 0; }))
        {
        }
        /// Constructor Construct from nullptr
        as_ptr()
            : ptr(nullptr)
        {
        }
        /// Copy Constructor Construct
        as_ptr(const as_ptr& other)
            : ptr(other.ptr)
            , con(ptr->OnDestroy.connect([this]() { ptr = 0; }))
        {
        }

        /// detach the ptr
        void detach()
        {
            if (ptr)
            {
                ptr = nullptr;
                con.disconnect();
            }
        }

        /// assign operator
        ///@param other the other async close ptr
        as_ptr< T >& operator=(const as_ptr< T >& other)
        {
            detach();
            new (this) as_ptr< T >(other);
            return std::ref(*this);
        }
        /// assign operator
        ///@param other pointer pointe to the async close class
        as_ptr< T >& operator=(T* other)
        {
            detach();
            new (this) as_ptr< T >(other);
            return std::ref(*this);
        }

        ///* operator
        T& operator*() const
        {
            assert(valid());
            return std::ref(*ptr);
        }
        ///-> operator
        T* operator->() const
        {
            assert(valid());
            return ptr;
        }

        /// valid function
        ///@return wether the ptr is not a nullptr
        bool valid() const
        {
            return ptr != nullptr;
        }

        /// get function
        ///@return the pointer it point to
        T* get() const
        {
            return ptr;
        }
        /// Destructor
        ~as_ptr()
        {
            detach();
        }
    };

    /// buffer class is a set of binary datas manage by refrence counter
    class BOOST_SYMBOL_EXPORT buffer
    {
        count_block*           ptr;
        size_t                 size_;
        mutable unsigned char* now_ptr;

      public:
        /// create a new buffer from the given size
        ///@param size_ the size of the buffer
        buffer(size_t size_);
        /// Copy Constructor
        /// the read_write pointer will be set to start
        buffer(const buffer& buf);
        /// begin funcion
        ///@return return the begin of the buffer
        unsigned char* begin();
        /// end funcion
        ///@return return the end of the buffer
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

        void print();
        ~buffer();
    };

    /*
        put uint16_t of host endian to the buffer
    */
    template <>
    void BOOST_SYMBOL_EXPORT buffer::put< uint16_t >(uint16_t a);

    /*
        put uint32_t of host endian to the buffer
    */
    template <>
    void BOOST_SYMBOL_EXPORT buffer::put< uint32_t >(uint32_t a);

    /*
        get uint16_t of host endian from the buffer
    */
    template <>
    uint16_t BOOST_SYMBOL_EXPORT buffer::get< uint16_t >() const;

    /*
        get uint32_t of host endian from the buffer
    */
    template <>
    uint32_t BOOST_SYMBOL_EXPORT buffer::get< uint32_t >() const;

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
