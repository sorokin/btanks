#ifndef __BTANKS_UTILS_H__
#define __BTANKS_UTILS_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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


#include <algorithm>
#include <functional>

template <class T> struct delete_ptr : public std::unary_function<T, void> {
	void operator()(const T &x) {
		delete x;
	}
};


template <class T> struct delete_ptr2 : public std::unary_function<T, void> {
	void operator()(T &x) {
		delete x.second;
		x.second = NULL;
	}
};

template <typename FuncPtr> union SharedPointer {
	FuncPtr call;
	void *ptr;
};


template <typename A, typename B, typename C> 
struct ternary {
	A first;
	B second;
	C third;
	
	ternary() : first(), second(), third() {}
	ternary(const A & a, const B &b, const C &c) : first(a), second(b), third(c) {}
};

template <typename A, typename B, typename C>
inline bool operator<(const ternary<A, B, C>& a, const ternary<A, B, C> &b) { 
	if (a.first < b.first)
		return true;
	if (b.first < a.first)
		return false;
	if (a.second < b.second)
		return true;
	if (b.second < a.second)
		return false;
	if (a.third < b.third)
		return true;
	return false;
}


#endif

