#ifndef BTANKS_MENU_CONTROL_H__
#define BTANKS_MENU_CONTROL_H__

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

#include "sdlx/sdlx.h"
#include "export_btanks.h"

namespace sdlx {
	class Surface;
}

class BTANKSAPI Control {
public: 
	Control();
	virtual void tick(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y) = 0;
	virtual void getSize(int &w, int &h) const = 0;
	
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);
	virtual ~Control() {}
	
	inline const bool changed() const { return _changed; } 
	inline void reset() { _changed = false; }
	
	inline void hide(const bool hide = true) { _hidden = hide; }
	inline const bool hidden() const { return _hidden; }
protected: 
	bool _changed;
	bool _hidden;
};

#endif

