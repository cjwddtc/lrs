#include "socket.h"
#include <algorithm>
#include <boost/config.hpp>
#include <memory>
#include <stdlib.h>
namespace lsy
{
	void as_mem_man::destroy(){
		OnDestroy();
		delete this;
	}
	void as_mem_man::bind_father(as_mem_man *fm){
		fm->OnDestroy.connect([this](){
			this->close();
		})
	}
    buffer::buffer(size_t size)
        : ptr((count_block*)malloc(size + sizeof(size_t)))
        , size_(size)
        , now_ptr(ptr->ptr)
    {
        ptr->count = 1;
    }

    buffer::buffer(const buffer& buf)
        : ptr(buf.ptr)
        , size_(buf.size_)
        , now_ptr(ptr->ptr)
    {
        ptr->count++;
    }

    void buffer::get(uint16_t& t) const
    {
        t = boost::asio::detail::socket_ops::network_to_host_short(
            *(uint16_t*)now_ptr);
        now_ptr += 2;
    }

    void buffer::get(uint32_t& t) const
    {
        t = boost::asio::detail::socket_ops::network_to_host_long(
            *(uint32_t*)now_ptr);
        now_ptr += 4;
    }

    void* buffer::data() const
    {
        return ptr->ptr;
    }

    void* buffer::data()
    {
        return ptr->ptr;
    }

    size_t buffer::size() const
    {
        return size_;
    }

    void buffer::resize(size_t size)
    {
        size_ = size;
    }

    size_t buffer::remain() const
    {
        return ptr->ptr + size_ - now_ptr;
    }

    size_t buffer::readed() const
    {
        return now_ptr - ptr->ptr;
    }

    void buffer::put(const unsigned char* ptr_, size_t size)
    {
        assert(now_ptr + size - ptr->ptr <= size_);
        std::copy(ptr_, ptr_ + size, now_ptr);
        now_ptr += size;
    }

    void buffer::put(const buffer& other)
    {
        size_t size = std::min(other.remain(), remain());
        put(other.now_ptr, size);
        other.now_ptr += size;
    }

    template <>
    void buffer::put< uint16_t >(uint16_t a)
    {
        *(uint16_t*)now_ptr = // a;
            boost::asio::detail::socket_ops::host_to_network_short(a);
        now_ptr += 2;
    }

    template <>
    void buffer::put< uint32_t >(uint32_t a)
    {
        uint32_t i = // a;
            boost::asio::detail::socket_ops::host_to_network_long(a);
        *(uint32_t*)now_ptr = i;

        now_ptr += 4;
    }

    unsigned char* buffer::get(size_t size) const
    {
        assert(now_ptr + size - ptr->ptr <= size_);
        unsigned char* p = now_ptr;
        now_ptr += size;
        return p;
    }

    template <>
    uint16_t buffer::get< uint16_t >() const
    {
        uint16_t t = *(uint16_t*)now_ptr;
        now_ptr += 2;
        return boost::asio::detail::socket_ops::network_to_host_short(t);
    }

    template <>
    uint32_t buffer::get< uint32_t >() const
    {
        uint32_t t = *(uint32_t*)now_ptr;
        now_ptr += 4;
        return boost::asio::detail::socket_ops::network_to_host_long(t);
    }

    void buffer::reset() const
    {
        now_ptr = ptr->ptr;
    }

    buffer::~buffer()
    {
        ptr->count--;

        if (!ptr->count)
            free(ptr);
    }
}

void lsy::buffer::get(unsigned char* ptr_, size_t size) const
{
    assert(now_ptr + size - ptr->ptr <= size_);
    std::copy(ptr_, ptr_ + size, now_ptr);
    now_ptr += size;
}

void lsy::buffer::get(const buffer& other) const
{
    size_t size = std::min(other.remain(), remain());
    get(other.now_ptr, size);
    other.now_ptr += size;
}

void lsy::buffer::renew(size_t new_size)
{
    if (ptr->count != 1)
    {
        ptr->count--;
        ptr        = (count_block*)malloc(new_size + sizeof(size_t));
        ptr->count = 1;
    }
    else
    {
        free(ptr);
        ptr        = (count_block*)malloc(new_size + sizeof(size_t));
        ptr->count = 1;
    }

    size_   = new_size;
    now_ptr = ptr->ptr;
}

void lsy::buffer::renew()
{
    if (ptr->count != 1)
    {
        ptr->count--;
        ptr        = (count_block*)malloc(size_ + sizeof(size_t));
        ptr->count = 1;
    }

    now_ptr = ptr->ptr;
}

unsigned char* lsy::buffer::begin()
{
    return ptr->ptr;
}

unsigned char* lsy::buffer::end()
{
    return ptr->ptr + size_;
}
