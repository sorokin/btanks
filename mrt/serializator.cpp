/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


#ifndef _WINDOWS
#	include <arpa/inet.h>
#else
#	include <winsock2.h>
#	ifndef snprintf
#		define snprintf _snprintf
#	endif
#endif

#include "serializator.h"
#include "chunk.h"
#include <assert.h>
#include <limits.h>
#include "exception.h"

#ifdef _WINDOWS
#	ifndef uint32_t
#		define uint32_t unsigned __int32
#	endif
#	ifndef uint16_t
#		define uint16_t unsigned __int16
#	endif
#endif


#define IEEE_754_SERIALIZATION

using namespace mrt;

#ifdef DEBUG
#define ASSERT_POS(size) assert(_pos + (size) <= _data->getSize())
#else
#define ASSERT_POS(size) if (_pos + (size) > _data->getSize()) \
	throw_ex(("buffer overrun %u + %u > %u", (unsigned)_pos, (unsigned)(size), (unsigned)_data->getSize()))
#endif

//ugly hackish trick, upcast const pointer to non-const variant.

Serializator::Serializator() : _data(new mrt::Chunk), _pos(0), _owns_data(true) {}
Serializator::Serializator(const mrt::Chunk *chunk) : _data((mrt::Chunk *)chunk), _pos(0), _owns_data(false) {}

Serializator::~Serializator() {
	if (_owns_data) {
		delete _data;
		_data = NULL;
	}
}

const bool Serializator::end() const {
	return _pos >= _data->getSize();
}

void Serializator::add(const int n) {
	//LOG_DEBUG(("added int %d", n));
	unsigned char buf[sizeof(unsigned long)];
	
	unsigned int x = (n >= 0)?n:-n;
	unsigned int len;
	unsigned char mask = (n >= 0)?0: 0x80;

	assert(x >= 0);
	
	//compact serialization for small numbers
	if (x <= 0x3f) { 
		unsigned char *ptr = (unsigned char *) _data->reserve(1) + _pos++;
		*ptr = mask | x;
		return;
	}

	mask |= 0x40;
	
	if (x <= 255) {
		buf[0] = x;
		len = 1;
	} else if (x <= 65535) {
		* (uint16_t *)buf = htons(x);
		len = sizeof(unsigned short);
	} else if (x <= 2147483647) {
		* (uint32_t *)buf = htonl(x); //defined as uint32 even on 64bit arch
		len = 4;
	} else throw_ex(("implement me (64bit values serialization)"));

	unsigned char *ptr = (unsigned char *) _data->reserve(1 + len) + _pos;
	*ptr++ = mask | len;
	memcpy(ptr, buf, len);
	_pos += len + 1;
}

void Serializator::add(const unsigned int n) {
	add((int)n);
}

void Serializator::get(unsigned int &n) const {
	int *p = (int *)&n;
	get(*p);
}

void Serializator::add(const bool b) {
	//LOG_DEBUG(("added bool %c", b?'t':'f'));
	add(b?1:0);
}

void Serializator::add(const std::string &str) {
	//LOG_DEBUG(("added string %s", str.c_str()));
	int size = str.size();
	add(size);
	
	unsigned char *ptr = (unsigned char *) _data->reserve(size) + _pos;
	memcpy(ptr, str.c_str(), size);
	_pos += size;
}

void Serializator::add(const Chunk &c) {
	int size = c.getSize();
	add(size);
	if (size == 0)
		return;

	unsigned char *ptr = (unsigned char *) _data->reserve(size) + _pos;
	memcpy(ptr, c.getPtr(), size);
	_pos += size;
}

void Serializator::add(const void *raw, const int size) {
	add(size);
	if (size == 0)
		return;

	unsigned char *ptr = (unsigned char *) _data->reserve(size) + _pos;
	memcpy(ptr, raw, size);
	_pos += size;
}

void Serializator::add(const float f) {
#ifdef IEEE_754_SERIALIZATION
	add(&f, sizeof(f));
#else
	//LOG_DEBUG(("added float %f", f));
	char buf[256];
	unsigned int len = snprintf(buf, sizeof(buf) -1, "%g", f);
	add(std::string(buf, len));
#endif
}

void Serializator::get(int &n)  const {
	unsigned char * ptr = (unsigned char *) _data->getPtr();

	ASSERT_POS(1);
	unsigned char type = *(ptr + _pos++);
	if ((type & 0x40) == 0) {
		n = type & 0x3f;
		if (type & 0x80) 
			n = -n;
		return;
	}

	unsigned char len = type & 0x3f;
	ASSERT_POS(len);
	
	if (len == 0) {
		n = 0; 
	} else if(len == 1) {
		n = *(ptr + _pos++);
	} else if (len == 2) {
		n = ntohs(*((uint16_t *)(ptr + _pos)));
		_pos += 2;
	} else if (len == 4) {
		n = ntohl(*((uint32_t *)(ptr + _pos)));
		_pos += 4;
//temp hack for 64 bit arch
	} else if (len == 8) {
		long nh = ntohl(*((uint32_t *)(ptr + _pos)));
		_pos += 4;
		long nl = ntohl(*((uint32_t *)(ptr + _pos)));
		_pos += 4;
		n = (nh << 32) | nl;
	} else 
		throw_ex(("control byte 0x%02x is unsupported. (corrupted data?) (position: %u, size: %u)", (unsigned)type, (unsigned)_pos, (unsigned)_data->getSize()));

	if (type & 0x80) 
		n = -n;
}

void Serializator::get(bool &b) const {
	int x;
	get(x);
	if (x != 0 && x != 1)
		throw_ex(("invalid boolean value '%02x'", x));
	b = x == 1;
}

void Serializator::get(float &f) const {
#ifdef IEEE_754_SERIALIZATION
	int size;
	get(size);
	if (size != sizeof(f))
		throw_ex(("failed to deserialize IEEE 754 float"));	
	get((void *)&f, size);
#else
	std::string str;
	get(str);
	if (sscanf(str.c_str(), "%f", &f) != 1)
		throw_ex(("failed to cast '%s' to float", str.c_str()));
#endif
}

void Serializator::get(std::string &str)  const {
	unsigned int size;
	get(size);

	ASSERT_POS(size);
	const char * ptr = (const char *) _data->getPtr() + _pos;
	str = std::string(ptr, size);
	_pos += size;
}

void Serializator::get(void *raw, const int size) const {
	ASSERT_POS(size);
	if (size == 0) 
		return;
	
	const char * ptr = (const char *) _data->getPtr() + _pos;
	memcpy(raw, ptr, size);
	_pos += size;
}


void Serializator::get(Chunk &c)  const {
	int size;
	get(size);

	ASSERT_POS(size);
	c.setSize(size);
	
	if (size == 0) 
		return;
	
	const char * ptr = (const char *) _data->getPtr() + _pos;
	memcpy(c.getPtr(), ptr, size);
	_pos += size;
}


const Chunk & Serializator::getData() const {
	return *_data;
}
