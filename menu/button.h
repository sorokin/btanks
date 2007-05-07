#ifndef BTANKS_MENU_Button_H__
#define BTANKS_MENU_Button_H__


/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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

namespace sdlx {
class Surface;
class Font;
}

#include <string>
#include "control.h"
#include "box.h"
#include "export_btanks.h"

class BTANKSAPI Button : public Control {
public: 
	Button(const std::string &font, const std::string &label);
	void getSize(int &w, int &h) const;
	virtual void render(sdlx::Surface& surface, const int x, const int y);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	
private: 
	int _w;
	Box _background;
	const sdlx::Font * _font;
	const std::string _label;
};

#endif

