#include "socket.h"
#include <stdlib.h>
namespace lsy{
buffer::buffer(size_t size):
		ptr((unsigned char *)malloc(size)),size_(size),
		now_ptr(ptr),count(new size_t){}


buffer::buffer(const buffer &buf) : ptr(buf.ptr),
		size_(buf.size_), now_ptr(buf.now_ptr),count(buf.count)
{
	(*count)++;
}

void buffer::get(uint16_t &t) const 
{
	t = *(uint16_t *)now_ptr;
	now_ptr += 2;
}

void buffer::get(uint32_t &t) const
{
	t = *(uint32_t *)now_ptr;
	now_ptr += 4;
}

void *buffer::data() const 
{
	return ptr;
}

void *buffer::data() 
{
	return ptr;
}

size_t buffer::size() const 
{
	return size_;
}

void buffer::resize(size_t size) 
{
	size_ = size;
}

void buffer::realloc(size_t size) 
{
	assert(ptr == now_ptr);
	ptr =(unsigned char * )::realloc(ptr, size);
	size_ = size;
	now_ptr = ptr;
}


size_t buffer::remain() const 
{
	return now_ptr - ptr;
}

void buffer::put(unsigned char *ptr, size_t size)
{
	std::copy(ptr, ptr + size, now_ptr);
	now_ptr += size;
}

unsigned char *buffer::get(size_t size) const 
{
	unsigned char *p = now_ptr;
	now_ptr += size;
	return p;
}

buffer::~buffer() {
	(*count)--;
	if(*count ==0){
		free(ptr);
		delete count;
	}
}
}