
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

#include "object.h"
#include "mrt/logger.h"
#include "resource_manager.h"

class Damage : public Object {
public:
	Damage();
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void onSpawn();
	//virtual void tick(const float dt);
	virtual Object * clone() const;
};


Damage::Damage() : Object("damage-digits") { 
	impassability = 0; 
	hp = -1;
	setDirectionsNumber(10);
}
Object * Damage::clone() const { return new Damage(*this); } 

void Damage::onSpawn() { 
	play("main", true); 
	_state.up = true; 
} 
/*
void Damage::tick(const float dt) {
		
}
*/

void Damage::render(sdlx::Surface &surface, const int x, const int y) {
	int digits = 1;
	int mult = 1;
	int n;
	
	for(n = hp; n >= 10; n/=10) {
		++digits;
		mult *= 10;
	}
	//LOG_DEBUG(("number: %d, digits = %d, mult: %d", hp, digits, mult));
	int xp = x;
	n = hp;
	while(digits--) {
		int d = n / mult;
		
		n %= mult;
		mult /= 10;
		//LOG_DEBUG(("digit %d", d));
		setDirection(d);
		Object::render(surface, xp, y);
		xp += (int)size.x;
	}
}

REGISTER_OBJECT("damage-digits", Damage, ());
