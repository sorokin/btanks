/*
MIT License

Copyright (c) 2008-2019 Netive Media Group & Vladimir Menshakov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef CLUNK_V3_H__
#define CLUNK_V3_H__

#include <clunk/types.h>
#include <limits>

namespace clunk {
/*! 
	\brief 3d vector 
	\tparam T type of the axis. usually int or float. 
*/
template <typename T> struct v3  {
	typedef std::numeric_limits<T> limits_type;

	template<typename V>
	static inline V abs(V v) { return v < 0? -v: v; }

	///x component
	T x;
	///y component
	T y;
	///z component
	T z;
	///default ctor: initializes all components with zeroes.
	inline v3<T>() : x(0), y(0), z(0) {}
	///initializes all components with given values
	inline v3<T>(const T x, const T y, const T z) : x(x), y(y), z(z) {} 
	
	///nullify vector
	inline void clear() { x = y = z = 0; }
	///returns true if x == y == z == 0 ? 
	inline bool is0() const {
		T zero = limits_type::epsilon();
		return abs(x) <= zero && abs(y) <= zero && abs(z) <= zero;
	}

	/*! 
		\brief normalizes vector. 
		\return old length of this vector
	*/ 
	inline T normalize() {
		const T len = length();
		if (len == (T)0 || len ==(T)1) 
			return len;
		
		x /= len;
		y /= len;
		z /= len;
		return len;
	}

	/*! 
		\brief normalizes vector with given length
		\param[in] nlen length
		\return old length of this vector
	*/ 

	inline T normalize(const T nlen) {
		const T len = length();
		if (len == (T)0 || len == nlen) 
			return len;
		
		x *= nlen / len;
		y *= nlen / len;
		z *= nlen / len;
		return len;
	}
	
	inline T dot_product(const v3<T> &v) const {
		return x * v.x + y * v.y + z * v.z;
	}

	inline v3<T> cross_product(const v3<T> &v) const {
		return v3<T>(
			y * v.z - z * v.y,
			z * v.x - x * v.z,
			x * v.y - y * v.x
		);
	}
	
	///returns length of this vector
	inline T length() const {
		const T ql = x * x + y * y + z * z;
		if (ql == (T)0 || ql == (T)1)
				return ql;

		return (T)sqrt(ql);
	}
	///returns square of length. To avoid sqrt if needed.
	inline T quick_length() const {
		return (T)(x * x + y * y + z * z);
	}

	///converts to vector of another type
	template <typename T2> 
		inline v3<T2> convert() const { return v3<T2>((T2)x, (T2)y, (T2)z); }

	///returns distance between two points	
	inline T distance(const v3<T>& other) const {
		v3<T>d(*this);
		d -= other;
		return d.length();
	}
	
	///return square distance between two points
	inline T quick_distance(const v3<T>& other) const {
		const T dx = x - other.x;
		const T dy = y - other.y;
		const T dz = z - other.z;
		return (dx * dx + dy * dy + dz * dz);
	}

	///allows v3 be placed in sorted STL container such std::map
	inline bool operator<(const v3<T> &other) const {
		if (x != other.x) {
			return x < other.x;
		}
		if (y != other.y) {
			return y < other.y;
		} 
		return z < other.z;
	}

	///negate all components
	inline v3<T> operator-() const {
		return v3<T>(-x, -y, -z);
	}
	///test equality 
	inline bool operator==(const v3<T> &other) const {
		return x == other.x && y == other.y && z == other.z;
	}

	///test inequality 
	inline bool operator!=(const v3<T> &other) const {
		return x != other.x || y != other.y || z != other.z;
	}
	
	///adds another vector
	inline v3<T>& operator+=(const v3<T>& other) {
		x += other.x; y += other.y; z += other.z;
		return *this;
	}

	///substracts another vector
	inline v3<T>& operator-=(const v3<T>& other) {
		x -= other.x; y -= other.y; z -= other.z;
		return *this;
	}

	///multiplies another vector
	inline v3<T>& operator*=(const v3<T>& other) {
		x *= other.x; y *= other.y; z *= other.z;
		return *this;
	}

	///divide with another vector
	inline v3<T>& operator/=(const v3<T>& other) {
		x /= other.x; y /= other.y; z /= other.z;
		return *this;
	}
	///multiplication
	inline v3<T> operator*(const v3<T>& other) const {
		return v3<T>(x * other.x, y * other.y, z * other.z);
	}
	///summing
	inline v3<T> operator+(const v3<T>& other) const {
		return v3<T>(x + other.x, y + other.y, z + other.z);
	}
	///substraction
	inline v3<T> operator-(const v3<T>& other) const {
		return v3<T>(x - other.x, y - other.y, z - other.z);
	}
	///division
	inline v3<T> operator/(const v3<T>& other) const {
		return v3<T>(x / other.x, y / other.y, z / other.z);
	}
	///multiplies all components with constant
	inline v3<T> operator*(const T& other) const {
		return v3<T>(x * other, y * other, z * other);
	}
	///sums all components with constant
	inline v3<T> operator+(const T& other) const {
		return v3<T>(x + other, y + other, z + other);
	}
	///substracts all components with constant
	inline v3<T> operator-(const T& other) const {
		return v3<T>(x - other, y - other, z - other);
	}
	///divides all components by constant
	inline v3<T> operator/(const T& other) const {
		return v3<T>(x / other, y / other, z / other);
	}
	///divides this vector by constant
	inline v3<T>& operator/=(const T& other) {
		x /= other;
		y /= other;
		z /= other;
		return *this;
	}

	///multiplies this vector with constant
	inline v3<T>& operator*=(const T& other) {
		x *= other;
		y *= other;
		z *= other;
		return *this;
	}

	///sums this vector with constant
	inline v3<T>& operator+=(const T& other) {
		x += other;
		y += other;
		z += other;
		return *this;
	}

	///substracts this vector with constant
	inline v3<T>& operator-=(const T& other) {
		x -= other;
		y -= other;
		z -= other;
		return *this;
	}

	///adds constant to the vector
	inline v3<T> operator+(const T a)  {
		return v3<T>(x + a, y + a, z + a);
	}

	///subs constant from the vector
	inline v3<T> operator-(const T a)  {
		return v3<T>(x - a, y - a, z - a);
	}

	///muls constant to the vector
	inline v3<T> operator*(const T a)  {
		return v3<T>(x * a, y * a, z * a);
	}

	///divs constant to the vector
	inline v3<T> operator/(const T a)  {
		return v3<T>(x / a, y  / a, z / a);
	}
};
///external operator+
template <typename T>
	inline v3<T> operator+(const T a, const v3<T> &v)  {
		return v3<T>(v.x + a, v.y + a, v.z + a);
	}

///external operator-
template <typename T>
	inline v3<T> operator-(const T a, const v3<T> &v)  {
		return v3<T>(a - v.x, a - v.y, a - v.z);
	}

///external operator*
template <typename T>
	inline v3<T> operator*(const T a, const v3<T> &v)  {
		return v3<T>(v.x * a, v.y * a, v.z * a);
	}

///external operator/
template <typename T>
	inline v3<T> operator/(const T a, const v3<T> &v)  {
		return v3<T>(a / v.x, a / v.y, a / v.z);
	}

typedef v3<float> v3f;

} //namespace clunk

#endif

