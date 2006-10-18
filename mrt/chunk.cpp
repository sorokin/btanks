#include "chunk.h"
#include <stdlib.h>
#include <string.h>
#include "ioexception.h"
#include "fmt.h"

using namespace mrt;

Chunk::Chunk(): ptr(NULL), size(0) {}

Chunk::Chunk(const int size) : ptr(NULL), size(0) {
	setSize(size);
}

void Chunk::fill(const int b) {
	if (ptr == NULL) 
		return;
	memset(ptr, b, size);
}

const Chunk& Chunk::operator=(const Chunk& c) {
    free();
    if (c.ptr == NULL || this == &c) 
    	return *this;
    assert(c.size > 0);
    
    if ((ptr = malloc(c.size)) == NULL) 
		throw_io(("malloc"));
    size = c.size;
    memcpy(ptr, c.ptr, c.size);
    return *this;
}

Chunk::Chunk(const Chunk& c) : ptr(NULL), size(0) { //fix it if you want to.
	*this = c;
}


void Chunk::setSize(size_t s) {
	if (s == 0) {
		free();
		return;
	}

	void * x = realloc(ptr, s);
	if (x == NULL) 
		throw_io(("realloc (%p, %d)", ptr, s));
	ptr = x;
	size = s;
}

void Chunk::setData(const void *p, const size_t s) {
	if (p == NULL || s == 0)
		throw_ex(("calling setData(%p, %u) is invalid", p, s));

	void *x = realloc(ptr, s);

	if (x == NULL) 
		throw_io(("realloc (%p, %d)", ptr, s));
	ptr = x;
	memcpy(ptr, p, s);
	size = s;
}

void Chunk::setData(void *p, const size_t s, const bool own) {
	if (p == NULL || s == 0) 
		throw_ex(("calling setData(%p, %u, %s) is invalid", p, s, own?"true":"false"));
	
	if (own) {
		free();
		ptr = p;
		size = s;
	} else {
		void *x = realloc(ptr, s);
		if (x == NULL) 
			throw_io(("realloc(%p, %d)", ptr, s));
		ptr = x;
		size = s;
		memcpy(ptr, p, s);
	}
}

void Chunk::append(const Chunk &other) {
	size_t s1 = size, s2 = other.getSize();
	if (s2 == 0)
		return;
	setSize(s1 + s2);
	memcpy((char *) ptr + s1, other.ptr, s2);
}

void* Chunk::reserve(const int more) {
	setSize(size + more);
	return ptr;
}

void Chunk::free() {
	if (ptr != NULL) {
		::free(ptr);
		ptr = NULL;
		size = 0;
	}
}

Chunk::~Chunk() {
	free();
}

const std::string Chunk::dump() const {
	if (ptr == NULL)
		return "empty memory chunk";
	assert(ptr != 0);
	
	std::string result = formatString("-[memory dump]-[size: %d]---", size);
	size_t n = (size - 1)/ 16 + 1;
	for(size_t i = 0; i < n; ++i) {
		result += "\n";
		size_t j, m = (size - i * 16);
		if (m > 16) 
			m = 16;
		
		for(j = 0; j < m; ++j) {
			const unsigned char *p = ((unsigned char *)ptr) + i*16 + j;
			result += formatString("%02x ", *p);
			if (j == 7) 
				result += " ";
		}
		for(; j < 16; ++j) {
			if (j == 7) 
				result += " ";
			result += "   ";
		}
		result += "\t\t";

		for(j = 0; j < m; ++j) {
			const unsigned char *p = ((unsigned char *)ptr) + i*16 + j;
			result += formatString("%c", (*p>=32 && *p < 128)? *p: '.');
			if (j == 7) 
				result += " ";
		}
		for(; j < m; ++j) {
			if (j == 7) 
				result += " ";
			result += " ";
		}
	}
	return result;
}
