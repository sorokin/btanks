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
#include "fmt.h"
#include "chunk.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <algorithm>
#include "exception.h"

using namespace mrt;

#define FORMAT_BUFFER_SIZE 1024

//#include "logger.h"

const std::string mrt::format_string(const char *fmt, ...) {
	va_list ap;

	//try static buffer on the stack to avoid malloc calls
	char static_buf[FORMAT_BUFFER_SIZE];

    va_start(ap, fmt);    
   	int r = vsnprintf (static_buf, FORMAT_BUFFER_SIZE - 1, fmt, ap);
    va_end(ap);

    if (r > -1 && r <= FORMAT_BUFFER_SIZE) 
   		return std::string(static_buf, r);

	int size = FORMAT_BUFFER_SIZE * 2;

	mrt::Chunk buf;

    while(true) {
		buf.set_size(size);
	    va_start(ap, fmt);    
    	int r = vsnprintf ((char *)buf.get_ptr(), size - 1, fmt, ap);
	    va_end(ap);
	    if (r > -1 && r <= size) 
    		return std::string((char *)buf.get_ptr(), r);
    	size *= 2;
    }
}

void mrt::trim(std::string &str, const std::string chars) {
	size_t i = str.find_first_not_of(chars);
	if (i > 0)
		str.erase(0, i);
	
	i = str.find_last_not_of(chars);
	if (i != str.npos)
		str.erase(i + 1, str.size());
}

void mrt::join(std::string &result, const std::vector<std::string>& array, const std::string &delimiter, const size_t limit) {
	result.clear();
	if (array.empty())
		return;
	
	size_t n = array.size();
	if (limit > 0 && limit < n) 
		n = limit;
	--n;
	for(size_t i = 0; i < n; ++i) {
		result += array[i];
		result += delimiter;
	}
	result += array[n];
}


void mrt::split(std::vector<std::string> & result, const std::string &str, const std::string &delimiter, const size_t limit) {
	result.clear();
	
	std::string::size_type pos = 0, p;
	size_t n = limit;
	
	while(pos < str.size()) {
		do {
			p = str.find(delimiter, pos);
			if (p == pos) {
				result.push_back(std::string());
				p += delimiter.size();
				pos += delimiter.size();
			}
		} while(p < str.size() && p == pos);
		
		
		if (p != std::string::npos) 
			result.push_back(str.substr(pos, p - pos));
		else {
			result.push_back(str.substr(pos));
			break;
		}
		if (n > 0) {
			if (--n == 0) {
				result[result.size() - 1] += str.substr(p);
				break;
			}
		}
		pos = p + delimiter.size();
	}
	if (limit)
		result.resize(limit);
}

void mrt::to_upper(std::string &str) {
	std::transform(str.begin(), str.end(), str.begin(), toupper);
}

void mrt::to_lower(std::string &str) {
	std::transform(str.begin(), str.end(), str.begin(), tolower);
}

void mrt::replace(std::string &str, const std::string &from, const std::string &to, const size_t limit) {
	if (str.empty())
		return;
	
	if (from.empty())
		throw_ex(("replace string must not be empty"));
	
	std::string::size_type pos = 0;
	size_t n = limit;

	while(pos < str.size()) {
		pos = str.find(from, pos);
		if (pos == str.npos) 
			break;
	
		str.replace(pos, from.size(), to);
		pos += from.size() - to.size() + 1;
	
		if (n > 0) {
			if (--n == 0) {
				break;
			}
		}
	}	
}
