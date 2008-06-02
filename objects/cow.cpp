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
#include "alarm.h"
#include "config.h"
#include "mrt/random.h"
#include "ai/herd.h"
#include "special_owners.h"

class Cow : public Object, public ai::Herd{
public:
	Cow(const std::string &classname) : 
		Object(classname), _reaction(true) {}
	
	virtual void tick(const float dt);
	virtual void calculate(const float dt);

	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, Object * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_reaction);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_reaction);
	}	

	virtual void onIdle(const float dt);
	const int getComfortDistance(const Object *other) const;

private: 
	Alarm _reaction;
};

const int Cow::getComfortDistance(const Object *other) const {
	GET_CONFIG_VALUE("objects.cow.comfort-distance", int, cd, 200);
	return (other == NULL || other->registered_name == registered_name)?cd:-1; //fixme names if you want
}


void Cow::onIdle(const float dt) {
	int tt;
	Config->get("objects." + registered_name + ".targeting-range", tt, 400);
	ai::Herd::calculateV(_velocity, this, 0, tt);
}


void Cow::calculate(const float dt) {
	if (_reaction.tick(dt) && !has_effect("panic"))
		onIdle(dt);

	GET_CONFIG_VALUE("objects.cow.rotation-time", float, rt, 0.2);
	limitRotation(dt, rt, true, false);
}

void Cow::tick(const float dt) {
	Object::tick(dt);

	if (_velocity.is0()) {
		if (getState() != "hold") {
			cancelAll();
			play("hold", true);
		}
	} else {
		if (getState() == "hold") {
			cancelAll();
			play("walk", true);
		}		
	}
}

void Cow::onSpawn() {
	float rt, drt = 1;
	
	Config->get("objects." + registered_name + ".reaction-time", rt, drt);
	mrt::randomize(rt, rt/10);
	_reaction.set(rt);
	play("hold", true);
	
	removeOwner(OWNER_MAP);
}

void Cow::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-cow", v2<float>(), v2<float>());
	} else if (emitter != NULL && emitter->piercing && event == "collision") {
		v2<float> v; 
		emitter->get_velocity(v);
		int dirs = getDirectionsNumber();
		int dir = v.getDirection(dirs);
		dir = (dirs + dir + dirs / (mrt::random(2)?4:-4)) % dirs;
		setDirection(dir);
		_velocity.fromDirection(dir, dirs);
		_direction = _velocity;
		add_effect("panic", 3.0f);
	}
	Object::emit(event, emitter);
}


Object* Cow::clone() const  {
	return new Cow(*this);
}

REGISTER_OBJECT("cow", Cow, ("creature"));
