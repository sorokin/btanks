#ifndef __BTANKS_V3_H__
#define __BTANKS_V3_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/


#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "mrt/serializator.h"
#include "mrt/serializable.h"

#include <string>
#include <stdexcept>
#include "mrt/exception.h"
#include <typeinfo>

template <typename T> class v3 : public mrt::Serializable {
public:
	T x, y, z;
	inline v3<T>() : x(0), y(0), z(0) {}
	inline v3<T>(const T x, const T y, const T z) : x(x), y(y), z(z) {} 

	inline void clear() { x = y = z = 0; }
	inline const bool is0() const = delete;
	
	inline const T normalize() {
		const T len = length();
		if (len == (T)0 || len ==(T)1) 
			return len;
		
		x /= len;
		y /= len;
		z /= len;
		return len;
	}

	inline const T normalize(const T nlen) {
		const T len = length();
		if (len == (T)0 || len == nlen) 
			return len;
		
		x *= nlen / len;
		y *= nlen / len;
		z *= nlen / len;
		return len;
	}
	
	inline const T length() const = delete;

	inline const T quick_length() const = delete;

	template <typename T2> 
		inline v3<T2> convert() const { return v3<T2>((T2)x, (T2)y, (T2)z); }
	
	inline const T distance(const v3<T>& other) const {
		v3<T>d(*this);
		d-= other;
		return d.length();
	}
	
	
	inline const T quick_distance(const v3<T>& other) const = delete;
	inline const T scalar(const v3<T> &other) const = delete;

	inline const bool same_sign(const v3<T> &other) const = delete;
	
	//operators 
	inline const bool operator<(const v3<T> &other) const {
		if (x != other.x) {
			return x < other.x;
		}
		if (y != other.y) {
			return y < other.y;
		} 
		return z < other.z;
	}


	inline const v3<T> operator-() const {
		return v3<T>(-x, -y, -z);
	}
	
	inline const bool operator==(const v3<T> &other) const {
		return x == other.x && y == other.y && z == other.z;
	}

	inline const bool operator!=(const v3<T> &other) const {
		return x != other.x || y != other.y || z != other.z;
	}
	

	inline const v3<T>& operator+=(const v3<T>& other) {
		x += other.x; y += other.y; z += other.z;
		return *this;
	}

	inline const v3<T>& operator-=(const v3<T>& other) {
		x -= other.x; y -= other.y; z -= other.z;
		return *this;
	}

	inline const v3<T>& operator*=(const v3<T>& other) {
		x *= other.x; y *= other.y; z *= other.z;
		return *this;
	}

	inline const v3<T>& operator/=(const v3<T>& other) {
		x /= other.x; y /= other.y; z /= other.z;
		return *this;
	}
	
	inline const v3<T> operator*(const v3<T>& other) const {
		return v3<T>(x * other.x, y * other.y, z * other.z);
	}
	inline const v3<T> operator+(const v3<T>& other) const {
		return v3<T>(x + other.x, y + other.y, z + other.z);
	}
	inline const v3<T> operator-(const v3<T>& other) const {
		return v3<T>(x - other.x, y - other.y, z - other.z);
	}
	inline const v3<T> operator/(const v3<T>& other) const {
		return v3<T>(x / other.x, y / other.y, z / other.z);
	}

	inline const v3<T> operator*(const T& other) const {
		return v3<T>(x * other, y * other, z * other);
	}
	inline const v3<T> operator+(const T& other) const {
		return v3<T>(x + other, y + other, z + other);
	}
	inline const v3<T> operator-(const T& other) const {
		return v3<T>(x - other, y - other, z - other);
	}
	inline const v3<T> operator/(const T& other) const {
		return v3<T>(x / other, y / other, z / other);
	}

	inline const v3<T>& operator/=(const T& other) = delete;

	inline const v3<T>& operator*=(const T& other) = delete;

	inline const v3<T>& operator+=(const T& other) = delete;

	inline const v3<T>& operator-=(const T& other) = delete;

public:	

	virtual void serialize(mrt::Serializator &s) const {
		s.add(x);
		s.add(y);
		s.add(z);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		s.get(x);
		s.get(y);
		s.get(z);
	}

	void fromString(const std::string &str) {
		clear();
		if (typeid(T) == typeid(int)) {
			if (sscanf(str.c_str(), "%d,%d,%d", &x, &y, &z) < 2)
				throw std::invalid_argument("cannot parse %d,%d,%d from " + str);
		} else throw std::invalid_argument("invalid type T. only int allowed for fromString()");
	}
};
	
template <typename T>
	const v3<T> operator+(const T a, const v3<T> &v) = delete;

template <typename T>
	const v3<T> operator+(const v3<T> &v, const T a) = delete;


template <typename T>
	const v3<T> operator*(const T a, const v3<T> &v) = delete;

template <typename T>
	const v3<T> operator*(const v3<T> &v, const T a) = delete;

template <typename T>
	const v3<T> operator/(const T a, const v3<T> &v) = delete;

template <typename T>
	const v3<T> operator/(const v3<T> &v, const T a) = delete;

#endif

