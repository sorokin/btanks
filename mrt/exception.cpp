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

#include "exception.h"
#include <stdarg.h>

using namespace mrt;

Exception::Exception() : _error() {}
Exception::~Exception() throw() {}

const std::string Exception::get_custom_message() { return std::string(); }
const char* Exception::what() const throw() { return _error.c_str(); }


void Exception::add_message(const char * file, const int line) {
	char buf[1024];
	size_t n = snprintf(buf, sizeof(buf), "[%s:%d]", file, line);
	_error = std::string(buf, n);
}

void Exception::add_message(const std::string &msg) {
	if (msg.empty()) return;
	
	_error += ": " + msg;
}
