#ifndef __BTANKS_CONTROL_METHOD_H__
#define __BTANKS_CONTROL_METHOD_H__

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


#include "player_state.h"
#include "alarm.h"

class PlayerSlot;

class ControlMethod {
protected:
	virtual void _updateState(PlayerSlot &slot, PlayerState &state) = 0;
public:	
	ControlMethod();
	void updateState(PlayerSlot &slot, PlayerState &state, const float dt);
	virtual void probe() const = 0;
	virtual ~ControlMethod() {}
private: 
	bool _release_set;
	Alarm _release_timer;
	PlayerState _old_state;
};

#endif

