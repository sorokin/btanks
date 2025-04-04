#ifndef __BTANKS_MATRIX_H__
#define __BTANKS_MATRIX_H__

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


#include "mrt/chunk.h"
#include "mrt/exception.h"

template <class T> class Matrix {
public:
	Matrix() : _data(), _w(0), _h(0), _use_default(false), _default() {}
	Matrix(const int h, const int w, const T v = 0): _use_default(false) {
		set_size(h, w, v);
	}
	
	void useDefault(const T d) { _default = d; _use_default = true; }
	
	void fill(const T v) {
		T *ptr = (T*) _data.get_ptr();
		for(int i = 0; i < _w * _h; ++i) {
			*ptr++ = v;
		}	
	}
	
	void set_size(const int h, const int w, const T v = 0) {
		_w = w;
		_h = h;
		_data.set_size(w * h * sizeof(T));

		fill(v);
	}
	
	inline const T get(const int y, const int x) const {
		if (x < 0 || x >= _w || y < 0 || y >= _h) {
			if (_use_default) 
				return _default;
			
			throw_ex(("get(%d, %d) is out of bounds", y, x));
		}
		int idx = y * _w + x;
		const T *ptr = (const T*) _data.get_ptr();
		return *(ptr + idx);
	}
	
	inline void set(const int y, const int x, const T v) {
		if (x < 0 || x >= _w || y < 0 || y >= _h) {
			if (_use_default)
				return;
			throw_ex(("set(%d, %d) is out of bounds", y, x));
		}
		
		int idx = y * _w + x;
		T *ptr = (T*) _data.get_ptr();
		*(ptr + idx) = v;
	}
	
	inline const int get_width() const { return _w; }
	inline const int get_height() const { return _h; }
	
	const std::string dump() const {
	//fixme: add template functions for conversion int/float other types to string
		std::string result;
		result += "      ";
		for(int x = 0; x < _w; ++x) 
			result += mrt::format_string("%-2d ", x);
		result += "\n";
		
		for(int y = 0; y < _h; ++y) {
			result += mrt::format_string("%-2d ", y);
			result += "[ ";
			for(int x = 0; x < _w; ++x) {
				result += mrt::format_string("%-2d ", (int)get(y, x));
			}
			result += " ]";
			result += mrt::format_string("%-2d\n", y);
		}

		result += "      ";
		for(int x = 0; x < _w; ++x) 
			result += mrt::format_string("%-2d ", x);
		result += "\n";

		return result;
	}
	
protected: 
	mrt::Chunk _data;	
	int _w, _h;
	bool _use_default;
	T _default;
};


#endif
