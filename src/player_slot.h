#ifndef __BTANKS_PLAYER_SLOT_H__
#define __BTANKS_PLAYER_SLOT_H__

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

#include "player_state.h"
#include "math/v3.h"
#include "sdlx/rect.h"
#include <string>

class Object;
class ControlMethod;

class PlayerSlot {
public:
	PlayerSlot();
	PlayerSlot(const int id);
	
	Object * getObject(); 
	const Object * getObject() const;

	int id;
	ControlMethod * control_method;
	v3<int> position;
		
	PlayerState state;
	bool need_sync;
	bool remote;
	float trip_time;
	
	bool visible;
	sdlx::Rect viewport;
		
	float mapx, mapy, mapvx, mapvy;
		
	void clear();
	~PlayerSlot();
		
	//respawn stuff.
	std::string classname;
	std::string animation;
	
	int frags;
};

#endif
