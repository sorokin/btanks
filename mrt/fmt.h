#ifndef __STACKVM__FMT_H__
#define __STACKVM__FMT_H__
/* M-Runtime for c++
 * Copyright (C) 2005-2006 Vladimir Menshakov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <string>
#include <vector>

#if !(defined(__GNUC__) || defined(__GNUG__) || defined(__attribute__))
#	define __attribute__(p) /* nothing */
#endif

namespace mrt {


const std::string formatString(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void trim(std::string &str, const std::string chars = "\t\n\r ");
void split(std::vector<std::string> & result, const std::string &str, const std::string &delimiter, const int limit = 0);

}

#endif

