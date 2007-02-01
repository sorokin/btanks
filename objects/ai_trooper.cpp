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

#include "trooper.h"
#include "ai/herd.h"
#include "config.h"
#include "resource_manager.h"
#include "mrt/random.h"

class AITrooper : public Trooper, ai::Herd {
public:
	AITrooper(const std::string &object, const bool aim_missiles) : 
		Trooper("trooper", object), _reaction(true), _target_dir(-1) {
			if (aim_missiles)
				_targets.insert("missile");
	
			_targets.insert("player");
			_targets.insert("trooper");
			_targets.insert("kamikaze");
			_targets.insert("boat");
	}
	virtual void onSpawn();
	virtual void serialize(mrt::Serializator &s) const {
		Trooper::serialize(s);
		_reaction.serialize(s);
		s.add(_target_dir);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Trooper::deserialize(s);
		_reaction.deserialize(s);
		s.get(_target_dir);
	}
	virtual void calculate(const float dt);
	virtual Object* clone() const;

	virtual void onIdle(const float dt);
	
private: 
	virtual const int getComfortDistance(const Object *other) const;

	Alarm _reaction;
	int _target_dir;
	
	//no need for serialize it:
	std::set<std::string> _targets;
};

const int AITrooper::getComfortDistance(const Object *other) const {
	GET_CONFIG_VALUE("objects.ai-trooper.comfort-distance", int, cd, 80);
	return (other == NULL || other->classname == "trooper")?cd:-1;
}

void AITrooper::onIdle(const float dt) {
	int summoner = getSummoner();
	if (summoner != 0 && summoner != -42) {
		float range = getWeaponRange(_object);
		ai::Herd::calculateV(_velocity, this, summoner, range);
	} else _velocity.clear();
	_state.fire = false;

	GET_CONFIG_VALUE("objects.trooper.rotation-time", float, rt, 0.05);
	limitRotation(dt, rt, true, false);
}

void AITrooper::onSpawn() {
	GET_CONFIG_VALUE("objects.trooper.reaction-time", float, rt, 0.1);
	mrt::randomize(rt, rt / 2);
	_reaction.set(rt);	
	Trooper::onSpawn();
}

Object* AITrooper::clone() const  {
	return new AITrooper(*this);
}


void AITrooper::calculate(const float dt) {
	//calculateWayVelocity();
	//LOG_DEBUG(("calculate"));
	if (!_reaction.tick(dt))
		return;
	if (getState() == "fire") {
		_state.fire = true; //just to be sure.
		return;
	}
	
	_state.fire = false;
	
	v3<float> vel;
	_target_dir = getTargetPosition(_velocity, _targets, _object);
	if (_target_dir >= 0) {
		//LOG_DEBUG(("target: %g %g %g", tp.x, tp.y, tp.length()));
		/*
		Way way;
		if (findPath(tp, way)) {
		setWay(way);
			calculateWayVelocity();
		}
		*/
		if (_velocity.length() >= 16) {
			quantizeVelocity();
			_direction.fromDirection(getDirection(), getDirectionsNumber());
		} else {
			_velocity.clear();
			setDirection(_target_dir);
			//LOG_DEBUG(("%d", _target_dir));
			_direction.fromDirection(_target_dir, getDirectionsNumber());
			_state.fire = true;
		}
	
	} else {
		_velocity.clear();
		_target_dir = -1;
		onIdle(dt);
	}
}
//==============================================================================
class TrooperInWatchTower : public Trooper {
public: 
	TrooperInWatchTower(const std::string &object, const bool aim_missiles) : 
		Trooper("trooper", object), _reaction(true) {
			if (aim_missiles)
				_targets.insert("missile");
	
			_targets.insert("player");
			_targets.insert("trooper");
			_targets.insert("kamikaze");
			_targets.insert("boat");		
	}
	virtual Object * clone() const { return new TrooperInWatchTower(*this); }
	
	virtual void onSpawn() { 
		GET_CONFIG_VALUE("objects.trooper.reaction-time", float, rt, 0.1);
		_reaction.set(rt);
	
		Trooper::onSpawn();
	}

	virtual void serialize(mrt::Serializator &s) const {
		Trooper::serialize(s);
		_reaction.serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Trooper::deserialize(s);
		_reaction.deserialize(s);
	}
	
	virtual void calculate(const float dt) {
		if (!_reaction.tick(dt))
			return;
		
		float range = getWeaponRange(_object);
		range *= range;
		//LOG_DEBUG(("range = %g", range));

		v3<float> pos, vel;
		if (getNearest(_targets, pos, vel) && pos.quick_length() <= range) {
			_state.fire = true;
			_direction = pos;
			_direction.normalize();
			setDirection(_direction.getDirection(getDirectionsNumber()) - 1);
			
		} else _state.fire = false;
	}
private: 
	Alarm _reaction; 

	//no need to serialize it
	std::set<std::string> _targets;
};

REGISTER_OBJECT("machinegunner", AITrooper, ("machinegunner-bullet", true));
REGISTER_OBJECT("thrower", AITrooper, ("thrower-missile", false));

REGISTER_OBJECT("machinegunner-in-watchtower", TrooperInWatchTower, ("machinegunner-bullet", true));
REGISTER_OBJECT("thrower-in-watchtower", TrooperInWatchTower, ("thrower-missile", false));
