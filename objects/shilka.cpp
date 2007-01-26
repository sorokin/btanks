
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

#include <assert.h>
#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "shilka.h"
#include "config.h"
#include "fakemod.h"


Shilka::Shilka(const std::string &classname) 
: Object(classname), _fire(false), _special_fire(false), _left_fire(true) {
}

FakeMod *Shilka::getMod(const std::string &name) {
	Object *o = get(name);
	assert(o != NULL);
	FakeMod *f = dynamic_cast<FakeMod*>(o);
	if (f == NULL)
		throw_ex(("cannot get FakeMod instance. [got %s(%s)]", o->registered_name.c_str(), o->classname.c_str()));
	return f;
}

void Shilka::onSpawn() {
	if (registered_name.substr(0, 6) == "static")
		disown();
	
	add("mod", spawnGrouped("fake-mod", "damage-digits", v3<float>::empty, Centered));
	
	Object *_smoke = spawnGrouped("single-pose", "tank-smoke", v3<float>::empty, Centered);
	_smoke->impassability = 0;

	add("smoke", _smoke);
	
	GET_CONFIG_VALUE("objects.shilka.fire-rate", float, fr, 0.2);
	_fire.set(fr);

	GET_CONFIG_VALUE("objects.shilka.special-fire-rate", float, sfr, 0.7);
	_special_fire.set(sfr);
}

Object * Shilka::clone() const {
	return new Shilka(*this);
}


void Shilka::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		if (registered_name != "ai-tank")
			World->detachVehicle(this);		
		
		cancelAll();
		//play("dead", true);
		spawn("corpse", "dead-" + animation);
		_velocity.x = _velocity.y = _velocity.z = 0;
		Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}


void Shilka::calculate(const float dt) {
	Object::calculate(dt);	
	GET_CONFIG_VALUE("objects.shilka.rotation-time", float, rt, 0.05);
	limitRotation(dt, rt, true, false);

	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}


void Shilka::tick(const float dt) {
	Object::tick(dt);

	const bool fire_possible = _fire.tick(dt);
	const bool special_fire_possible = _special_fire.tick(dt);
	
	if (getState().empty()) {
		play("hold", true);
	}

	_velocity.normalize();
	if (_velocity.is0()) {
		cancelRepeatable();
		play("hold", true);
	} else {
		if (getState() == "hold") {
			cancelAll();
			play("start", false);
			play("move", true);
		}
	}
	
	bool play_fire = false;

	if (_state.fire && fire_possible) {
		_fire.reset();
			
		static const std::string left_fire = "shilka-bullet-left";
		static const std::string right_fire = "shilka-bullet-right";
		std::string animation = "shilka-bullet-";
		animation += (_left_fire)?"left":"right";
		if (isEffectActive("ricochet")) {
			spawn("ricochet-bullet", "ricochet-bullet", v3<float>::empty, _direction);
			play_fire = true;
		} else if (isEffectActive("dispersion")) {
			if (special_fire_possible) {
				_special_fire.reset();
				spawn("dispersion-bullet", "dispersion-bullet", v3<float>::empty, _direction);
				play_fire = true;
				goto skip_left_toggle;
			};
		} else { 
			spawn("shilka-bullet", animation, v3<float>::empty, _direction);
			play_fire = true;
		}
		_left_fire = ! _left_fire;
	}

skip_left_toggle:

	if (_state.alt_fire && special_fire_possible) {
		_special_fire.reset();
		
		FakeMod * mod = getMod("mod");
		
		if (isEffectActive("dirt")) {
			if (getState().substr(0,4) == "fire") 
				cancel();
		
			static const std::string left_fire = "shilka-bullet-left";
			static const std::string right_fire = "shilka-bullet-right";
			std::string animation = "shilka-dirt-bullet-";
			animation += (_left_fire)?"left":"right";

			spawn("dirt-bullet", animation, v3<float>::empty, _direction);

			_left_fire = ! _left_fire;
			play_fire = true;
		} else if (!mod->getType().empty()) {
			if (mod->getCount() > 0) {
				spawn(mod->getType(), mod->getType(), _direction*(size.length()/-2), v3<float>::empty);
				mod->decreaseCount();
			}
		}
	}


	if (play_fire) {
		if (getState().substr(0,4) == "fire") 
			cancel();
		
		playNow(_left_fire?"fire-left":"fire-right");
	}
}

const bool Shilka::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "effects") {
		if (type == "dispersion") {
			removeEffect("ricochet");
		} else if (type == "ricochet") {
			removeEffect("dispersion");
		} else if (type == "dirt") {
			getMod("mod")->setType(std::string());
		}
		addEffect(type);
		return true;
	} else if (obj->classname =="mod") {
		if (type == "machinegunner" || type == "thrower") {
			removeEffect("dirt");
			FakeMod *mod = getMod("mod");
			mod->setType(type);
			int n;
			Config->get("objects.shilka." + type + "-capacity", n, 5);
			mod->setCount(n);
			return true;
		}
	}
	return BaseObject::take(obj, type);
}

void Shilka::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	_fire.serialize(s);
	_special_fire.serialize(s);
	s.add(_left_fire);
}
void Shilka::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	_fire.deserialize(s);
	_special_fire.deserialize(s);
	s.get(_left_fire);
}

REGISTER_OBJECT("shilka", Shilka, ("player"));
REGISTER_OBJECT("static-shilka", Shilka, ("vehicle"));
