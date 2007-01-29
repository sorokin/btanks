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

#include "destructable_object.h"
#include "config.h"
#include "resource_manager.h"
#include "world.h"

class Barrack : public DestructableObject {
public:
	Barrack(const std::string &object, const std::string &animation, const bool pierceable) : 
		DestructableObject("barrack", "fire", "fire", pierceable), 
		_object(object), _animation(animation), _spawn(true) {}

	virtual Object* clone() const  { return new Barrack(*this); }
	
	virtual void tick(const float dt);
	virtual void onSpawn();

	virtual void serialize(mrt::Serializator &s) const {
		DestructableObject::serialize(s);
		s.add(_object);
		s.add(_animation);
		_spawn.serialize(s);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		DestructableObject::deserialize(s);
		s.get(_object);
		s.get(_animation);
		_spawn.deserialize(s);
	}

private:
	std::string _object, _animation;
	Alarm _spawn;
};

void Barrack::tick(const float dt) {
	DestructableObject::tick(dt);

	static std::vector<std::string> targets;
	if (targets.empty()) {
		targets.push_back("player");
		targets.push_back("trooper");
		targets.push_back("kamikaze");
	}
	
	
	if (!_broken && _spawn.tick(dt)) {
		int tr;
		Config->get("objects." + registered_name + ".targeting-range", tr, 500);

		v3<float> pos, vel;
		if (getNearest(targets, pos, vel) && pos.length() >= tr)
			return; //skip spawning
		
		int max_c;
		Config->get("objects." + registered_name + ".maximum-children", max_c, 5);
		int n = World->getChildren(getID());
		if (n < max_c) {
			v3<float>dpos;
			dpos.y = size.y / 2 + 16; //fixme: use debiloids size here.
			
			spawn(_object, _animation, dpos);
			playNow("spawn");
		}
	}
}

void Barrack::onSpawn() {
	play("main", true);
	float sr;
	Config->get("objects." + registered_name + ".spawn-rate", sr, 5);
	_spawn.set(sr);
}



REGISTER_OBJECT("barrack-with-machinegunners", Barrack, ("machinegunner", "machinegunner", false));
REGISTER_OBJECT("barrack-with-throwers", Barrack, ("thrower", "thrower", false));
REGISTER_OBJECT("barrack-with-kamikazes", Barrack, ("kamikaze", "kamikaze", false));

REGISTER_OBJECT("tent-with-machinegunners", Barrack, ("machinegunner", "machinegunner", true));
REGISTER_OBJECT("tent-with-throwers", Barrack, ("thrower", "thrower", true));
REGISTER_OBJECT("tent-with-kamikazes", Barrack, ("kamikaze", "kamikaze", true));
