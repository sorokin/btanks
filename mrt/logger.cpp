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

#include "logger.h"
#include <stdio.h>
#include "ioexception.h"

#ifdef _WINDOWS
#	define WINDOWS_LEAN_AND_MEAN
#	include <windows.h>
#else
#	include <string.h>
#	include <sys/time.h>
#	include <time.h>
#endif

using namespace mrt;

IMPLEMENT_SINGLETON(mrt::Logger, ILogger);

ILogger::ILogger() : _level(LL_DEBUG), _lines(0), fd(NULL) {}

void ILogger::assign(const std::string &file) {
	close();
	fd = fopen(file.c_str(), "wt");	
	if (fd == NULL)
		throw_io(("fopen('%s', 'wt')", file.c_str()));
}

void ILogger::close() {
	if (fd != NULL) {
		fclose(fd);
		fd = NULL;
	}
}

ILogger::~ILogger() {
	close();
}

void ILogger::set_log_level(const int level) {
	_level = level;
}

const char * ILogger::get_log_level_name(const int level) {
	switch(level) {
		case LL_DEBUG: return "debug";
		case LL_NOTICE: return "notice";
		case LL_WARN: return "warn";
		case LL_ERROR: return "error";
		default: return "unknown";
	}
}


void ILogger::log(const int level, const char *file, const int line, const std::string &str) {
	if (level < _level) return;
	++_lines;
	int h, m, s, ms;
#ifdef _WINDOWS
	struct _SYSTEMTIME st;
	GetLocalTime(&st);
	h = st.wHour; m = st.wMinute; s = st.wSecond; ms = st.wMilliseconds;
#else
	struct timeval tv;
	memset(&tv, 0, sizeof(tv));
	gettimeofday(&tv, NULL);
	
	struct tm tm;
	localtime_r(&tv.tv_sec, &tm);
	
	h = tm.tm_hour;
	m = tm.tm_min;
	s = tm.tm_sec;
	ms = tv.tv_usec / 1000;
#endif
	fprintf(fd?fd:stderr, "[%02d:%02d:%02d.%03d][%s:%d]\t [%s] %s\n", h, m, s, ms, file, line, get_log_level_name(level), str.c_str());
}

