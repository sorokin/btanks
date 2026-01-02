/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
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

#include "timer.h"
#include "mrt/ioexception.h"

#include <thread>

using namespace sdlx;

Timer::Timer() {
}

Timer::~Timer() {
}


void Timer::reset() {
	reset_time_point = std::chrono::steady_clock::now();
}

const int Timer::microdelta() const {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - reset_time_point).count();
}


void Timer::microsleep(const char *why, const int micros) {
	std::this_thread::sleep_for(std::chrono::microseconds(micros));
}
