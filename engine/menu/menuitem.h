#ifndef __BT_MENUITEM_H__
#define __BT_MENUITEM_H__

/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include "sdlx/color.h"
#include "sdlx/surface.h"
#include <string>
#include <map>

namespace sdlx {
class Font;
}

class MenuItem {
public:
	const std::string name;
	const std::string type;
	
	MenuItem(const sdlx::Font *font, const std::string &name, const std::string &type, const std::string &text, const std::string &value = std::string());
	virtual void render(sdlx::Surface &dst, const int x, const int y) const;
	void get_size(int &w, int &h) const;

	virtual void onClick() {}

	virtual void onFocus();
	virtual void onLeave();

	virtual const bool onKey(const SDL_keysym sym);
	virtual const std::string getValue() const;
	virtual ~MenuItem() {}

protected:
	void render();
		
	std::string _text, _value;

private:
	const sdlx::Font * _font;
	sdlx::Surface _normal;
};

#endif
