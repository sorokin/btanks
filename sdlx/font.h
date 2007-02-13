#ifndef __SDLX_FONT_H__
#define __SDLX_FONT_H__

/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
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

namespace sdlx {

class Surface;
class Font {
public:
	enum Type { AZ09, Ascii };
	Font();
	~Font();
	
	void load(const std::string &file, const Type type, const bool alpha = true);
	const int getHeight() const;
	const int render(sdlx::Surface &window, const int x, const int y, const std::string &str) const;
	void clear();

private:
	Font(const Font &);
	const Font& operator=(const Font &);
	Type _type;
	sdlx::Surface *_surface;
};

}

#endif

