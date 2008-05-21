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

#include "object.h"
#include "registrar.h"

class ShilkaTurret : public Object {
public:
	void tick(const float dt) {
		Object::tick(dt);
	}
	
	ShilkaTurret() : Object("turrel") {
		impassability = 0;
		hp = -1;
		setDirectionsNumber(16);
		pierceable = true;
	}

	void calculate(const float dt) {
		Object::calculate(dt);
	}
	
	virtual Object * clone() const { return new ShilkaTurret(*this); }
	
	void onSpawn() {
		play("main", true);
	}

private:
};

REGISTER_OBJECT("shilka-turret", ShilkaTurret, ());
