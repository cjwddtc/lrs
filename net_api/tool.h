/*! @file
\brief this file is network tool for the net_api
mainly about buffer and close_api
*/
#pragma once
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
        T*                          ptr;
        boost::signals2::connection con;
        /// Constructor Construct from the pointer to the contain async class
        ///@param pointer to the contain async class
        as_contain(T* ptr_)
            : ptr(ptr_)
        {
            ptr->OnDestroy
                .connect([this]() {
                    boost::signals2::connection().swap(con);
                    delete this;
                })
                .swap(con);
        }

      protected:
        /// virtual Destructor
        virtual ~as_contain()
        {
            con.disconnect();
        }
    };
    /// async close ptr:safe pointer to the async close class
    /// it will be nullptr when the async close object is closed
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
        /// Constructor from std::string
        ///@param str the string
        buffer(const std::string& str);
        buffer(std::initializer_list< buffer > list);
        buffer(uint16_t n);
        buffer(uint32_t n);
        /// begin funcion
        ///@return return the begin of the buffer
        unsigned char* begin();
        /// end funcion
        ///@return return the end of the buffer
        unsigned char* end();

        /// get the raw data of the buffer
        ///@return return the ptr point to the raw data
        void* data() const;

        /// change the logical size of the buffer
        ///@warning this function will no change the real size of the buffer's
        /// data so size should be smaller than the real size of the buffer's
        /// data if you want to change the real size use renew
        ///@param size the logical size of the buffer
        void resize(size_t size);

        /// get the size of the buffer
        ///@return the size of the buffer
        size_t size() const;
        /// get the remain byte which is not read or write
        ///@return the remain byte which is not read or write
        size_t remain() const;
        /// get the remain byte which is read or write
        ///@return the remain byte which is read or write
        size_t readed() const;

        /// put uint16_t to the buffer by the network endian
        ///@param a the uint16_t
        void put(uint16_t a);
        /// put uint32_t to the buffer by the network endian
        ///@param a the uint32_t
        void put(uint32_t a);
        /// put a data to the buffer
        ///@param ptr point to the data
        ///@param size size of the data
        void put(const unsigned char* ptr, size_t size);
        /// put another buffer to the buffer
        ///@param buf another buffe
        void put(const buffer& buf);


        /// get a uint16_t from the buffer to the host endian
        ///@param t the uint16_t to save
        void get(uint16_t& t) const;
        /// get a uint32_t from the buffer to the host endian
        ///@param t the uint32_t to save
        void get(uint32_t& t) const;
        /// put a data from the buffer
        ///@param ptr save the data the to ptr
        ///@param size size of the data to get
        ///@warning size should smaller than remain()
        void get(unsigned char* ptr, size_t size) const;
        /// read data to another buffer as much as possible
        ///@param buf another buffe
        void get(buffer& buf) const;
        /// template return version of get
        ///@return the value get
        template < class T >
        T get() const
        {
            T a;
            get(a);
            return a;
        }
        /// get size of data from the buffer
        ///@param size the size of the data to get
        ///@return the ptr point to the data
        unsigned char* get(size_t size) const;

        /// reset the read_write point to the start
        void reset() const;
        /// realloc the memory of this buffer
        /// if the old memory is only own by this buffer it will be free orcount
        /// will be decrease
        ///@param the new size
        void renew(size_t new_size);
        /// realloc the memory which have same size as the previous buffer of
        /// this buffer
        /// if the old memory is only own by this buffer it will be free orcount
        /// will be decrease
        void renew();
        /// get the refrence count
        ///@return refrence count
        size_t count() const;
        /// print the buffer by hex
        void print() const;
        /// Destructor
        ~buffer();
    };
}
