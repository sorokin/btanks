#ifndef __BTANKS_MOUSE_CONTROL_H__
#define __BTANKS_MOUSE_CONTROL_H__

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


#include "sdlx/joystick.h"
#include "control_method.h"
#include "player_state.h"
#include "math/v2.h"

class Object;
class MouseControl : public ControlMethod {
public:
	MouseControl(); 
	virtual void updateState(PlayerState &state);
private:
	void getPosition(v2<float>&pos) const;
	Object * getObject() const;
	
	void onMouse(const int button, const bool pressed, const int x, const int y);
	v2<float> _target_rel, _target;
	int _target_dir;
	bool _shoot;
};

#endif
