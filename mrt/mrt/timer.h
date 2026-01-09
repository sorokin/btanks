#ifndef MRT_TIMER_H_
#define MRT_TIMER_H_

/* M-runtime for c++
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

#include "mrt/export_mrt.h"
#include <chrono>

namespace mrt {
class MRTAPI Timer {
public: 
	Timer();
	~Timer();

	void reset();
	const int microdelta() const;
	static void microsleep(const char *why, const int micros);
private:
	std::chrono::steady_clock::time_point reset_time_point;
};
}

#endif
