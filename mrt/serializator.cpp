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
#endif

#include "serializator.h"
#include "chunk.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "exception.h"

//#define IEEE_754_SERIALIZATION
//define it to use machine/compiler specific binary format 

#include <math.h>

using namespace mrt;

#ifdef DEBUG
#define ASSERT_POS(size) assert(_pos + (size) <= _data->get_size())
#else
#define ASSERT_POS(size) if (_pos + (size) > _data->get_size()) \
	throw_ex(("buffer overrun %u + %u > %u", (unsigned)_pos, (unsigned)(size), (unsigned)_data->get_size()))
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

bool Serializator::end() const {
	return _pos >= _data->get_size();
}

void Serializator::add(const int n) {
	//LOG_DEBUG(("added int %d", n));
	
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
	
	unsigned char buf[4];
	if (x <= 255) {
		buf[0] = x;
		len = 1;
	} else if (x <= 65535) {
		buf[0] = (x >> 8);
		buf[1] = x;
		len = 2;
	} else if (x <= 2147483647) {
		buf[0] = (x >> 24);
		buf[1] = (x >> 16);
		buf[2] = (x >> 8);
		buf[3] = x;
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
	int size = c.get_size();
	add(size);
	if (size == 0)
		return;

	unsigned char *ptr = (unsigned char *) _data->reserve(size) + _pos;
	memcpy(ptr, c.get_ptr(), size);
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
	{
		if (f == 0) {
			add(0);
			return;
		} else if (f == 1) {
			add(-4);
			return;
		} else if (f == -1) {
			add(-5);
			return;
		} else if (isnan(f)) {
			add(-1); //magic! :)
			return;
		} 
		int f_inf = isinf(f);
		if (f_inf != 0) {
			add(f_inf > 0? -2: -3);
			return; //magic values for nan 
		}
	}
	
	char buf[32];
	char num[8];
	int len = snprintf(buf, sizeof(buf), "%g", f);
	assert(len >= 0 && len < (int)sizeof(buf));
	memset(num, 0, sizeof(num));
	for(int i = 0; i < len; ++i) {
		char c = buf[i];
		int idx = -1;
		if (c >= '0' && c <= '9') {
			idx = 1 + c - '0';
		} else if (c == '.') {
			idx = 11;
		} else if (c == 'e' || c == 'E') {
			idx = 12;
		} else if (c == '-') {
			idx = 13;
		}
		assert(idx >= 0 && idx < 16);
		assert(i / 2 < (int)sizeof(num));
		num[i/2] |= (i & 1)?idx: idx << 4;
	}
	add(num, (len + 1) / 2);
#endif
}

void Serializator::get(int &n)  const {
	unsigned char * ptr = (unsigned char *) _data->get_ptr();

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
		n = uint8_t(ptr[_pos]);
		++_pos;
	} else if (len == 2) {
		n = uint16_t(ptr[_pos]) << 8
		  | uint16_t(ptr[_pos + 1]);
		_pos += 2;
	} else if (len == 4) {
		n = uint32_t(ptr[_pos]) << 24
		  | uint32_t(ptr[_pos + 1]) << 16
		  | uint32_t(ptr[_pos + 2]) << 8
		  | uint32_t(ptr[_pos + 3]);
		_pos += 4;
	} else 
		throw_ex(("control byte 0x%02x is unsupported. (corrupted data?) (position: %u, size: %u)", (unsigned)type, (unsigned)_pos, (unsigned)_data->get_size()));

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
		throw_ex(("failed to deserialize IEEE 754 float(size %d, need %u)", size, (unsigned)sizeof(float)));	
	get((void *)&f, size);
	//LOG_DEBUG(("%g", f));
#else
	int len;
	get(len);

	switch(len) {
	case 0: 
		f = 0;
		return;
	case -1: 
		f = NAN;
		return;
	case -2:
		f = INFINITY;
		return;
	case -3: 
		f = -INFINITY;
		return;
	case -4: 
		f = 1;
		return;
	case -5: 
		f = -1;
		return;
	}

	unsigned char buf[32];
	if (len >= (int)sizeof(buf))
		throw_ex(("float number too long(%d)", len));

	memset(buf, 0, sizeof(buf));
	get(buf, len);

	std::string num;
	for(int i = 0; i < len * 2; ++i) {
		int c = buf[i / 2];
		int d = ((i & 1)? c: c >> 4) & 0x0f;
		if (d == 0) {
			break;
		} else if (d >= 1 && d <= 10) {
			num += ('0' + d - 1);
		} else if (d == 11) {
			num += '.';
		} else if (d == 12) {
			num += 'e';
		} else if (d == 13) {
			num += '-';
		} else {
			throw_ex(("unknown float character %d", d));
		}
	}
	if (sscanf(num.c_str(), "%g", &f) != 1) 
		throw_ex(("failed to get float value from '%s'", num.c_str()));
#endif
}

void Serializator::get(std::string &str)  const {
	unsigned int size;
	get(size);

	ASSERT_POS(size);
	const char * ptr = (const char *) _data->get_ptr() + _pos;
	str = std::string(ptr, size);
	_pos += size;
}

void Serializator::get(void *raw, const int size) const {
	ASSERT_POS(size);
	if (size == 0) 
		return;
	
	const char * ptr = (const char *) _data->get_ptr() + _pos;
	memcpy(raw, ptr, size);
	_pos += size;
}


void Serializator::get(Chunk &c)  const {
	int size;
	get(size);

	ASSERT_POS(size);
	c.set_size(size);
	
	if (size == 0) 
		return;
	
	const char * ptr = (const char *) _data->get_ptr() + _pos;
	memcpy(c.get_ptr(), ptr, size);
	_pos += size;
}


const Chunk & Serializator::getData() const {
	return *_data;
}

void Serializator::finalize(mrt::Chunk &data) {
	if (_data->empty()) {
		data.free();
		return;
	}
	data.set_data(_data->get_ptr(), _data->get_size(), true);
	_data->unlink();
}
