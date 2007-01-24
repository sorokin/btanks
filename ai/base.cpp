
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
#include "base.h"
#include <assert.h>
#include "world.h"
#include "config.h"

#include "math/unary.h"

using namespace ai;

Base::Base() : Object("player"), _reaction_time(true), _refresh_path(true), _target_id(-1) {}

Base::~Base() {
	LOG_DEBUG(("traits: \n%s", _traits.save().c_str()));
}

void Base::addEnemyClass(const std::string &classname) {
	_enemies.insert(classname);
}

void Base::addBonusName(const std::string &rname) {
	_bonuses.insert(rname);
}


void Base::onSpawn() {
	const std::string vehicle = getType();
	if (vehicle.empty())
		throw_ex(("vehicle MUST provide its type"));
	
	LOG_DEBUG(("spawning as '%s'", vehicle.c_str()));
	if (_enemies.size() == 0 && _bonuses.size()) 
		throw_ex(("vehicle was not provide enemies/bonuses"));
		
	float rt, rpi;
	Config->get("objects.ai-" + vehicle + ".reaction-time", rt, 0.1);
	_reaction_time.set(rt);
	Config->get("objects.ai-" + vehicle + ".refresh-path-interval", rpi, 1);
	_refresh_path.set(rpi);
	Config->get("objects.ai-" + vehicle + ".pathfinding-slice", _pf_slice, 10);
}

const bool Base::isEnemy(const Object *o) const {
	return _enemies.find(o->classname) != _enemies.end();
}

const std::string Base::convertName(const std::string &weapon) {
	std::string wc, wt;
	std::string::size_type p;
	if ((p = weapon.rfind(':')) != std::string::npos) {
		wc = weapon.substr(0, p);
		wt = weapon.substr(p + 1);
	} else {
		wt = weapon;
	}
	if (wc.empty() || wt.empty()) 
		return std::string();
	return wt + "-" + wc.substr(0, wc.size() - 1);
}


const bool Base::checkTarget(const Object * target, const std::string &weapon) const {
	if (!isEnemy(target))
		return false;
	
	v3<float> pos = getRelativePosition(target);
	
	std::string wc, wt;
	{
		std::string::size_type p;
		if ((p = weapon.rfind(':')) != std::string::npos) {
			wc = weapon.substr(0, p);
			wt = weapon.substr(p + 1);
		} else {
			wt = weapon;
		}
	}

	//LOG_DEBUG(("moo(%s/%s): %g %g", wc.c_str(), wt.c_str(), pos.x, pos.y));
	
	bool codir, codir1;
	{
		v3<float> d(pos);
		d.normalize();
		int dir = d.getDirection(getDirectionsNumber()) - 1;
		codir = dir == getDirection();
		int dd = math::abs(dir - getDirection());
		codir1 = dd == 1 || dd == (getDirectionsNumber() - 1);
	}
	
	if (wc == "missiles") {
		if (codir)
			return true;
		if (wt == "guided" && codir1)
			return true;
	} else if (wc == "mines") {
		if (!_velocity.is0())
			return true;
	}
	return false;
}


void Base::calculate(const float dt) {
	const bool refresh_path = _refresh_path.tick(dt);
	const bool dumb = !_reaction_time.tick(dt);
	const Object *target = NULL;
	
	if (!refresh_path && dumb) 
		goto gogogo;

	{
	const std::string weapon1 = getWeapon(0), weapon2 = getWeapon(1);
	const int amount1 = getWeaponAmount(0), amount2 = getWeaponAmount(1);
	
	static const std::set<std::string> empty_enemies;
	
	target = World->findTarget(this, (amount1 > 0 || amount2 > 0)?_enemies:empty_enemies, _bonuses, _traits);
	if (target != NULL && ((refresh_path && isEnemy(target)) || target->getID() != _target_id)) {
		_target_id = target->getID();
		_enemy = isEnemy(target);

		if (_enemy && !weapon1.empty()) {
			v3<float> r;
			if (getTargetPosition(r, target->getPosition(), convertName(weapon1)))
				_target_position = r.convert<int>();
		}
		
		target->getCenterPosition(_target_position);
		_target_position -= (size / 2).convert<int>();
		LOG_DEBUG(("next target: %s at %d,%d", target->registered_name.c_str(), _target_position.x, _target_position.y));
		findPath(_target_position, 16);
		//Way way;
		//if (!old_findPath(target, way))
		//	LOG_WARN(("no way"));
		//else setWay(way);

	}

	//2 fire or not 2 fire.

	if (target == NULL)
		target = World->getObjectByID(_target_id);
	if (target == NULL)
		goto gogogo;

	if (!weapon1.empty())
		_state.fire = checkTarget(target, weapon1);
	if (!weapon2.empty())
		_state.alt_fire = checkTarget(target, weapon2);

	}
	
	//LOG_DEBUG(("w1: %s", getWeapon(0).c_str()));
	//LOG_DEBUG(("w2: %s", getWeapon(1).c_str()));
	
	//bool driven = isDriven();
	
	//LOG_DEBUG(("calculating: %c, driven: %c", calculating?'+':'-', driven?'+':'-'));
	gogogo:
	
	Way way;
	
	if (calculatingPath()) {
		int n = 1;
		bool found;
		while(! (found = findPathDone(way)) && n < _pf_slice)
			++n;
		
		if (found) {
			//LOG_DEBUG(("n = %d", n));
			if (!way.empty()) {
				setWay(way);
			} else {
				LOG_WARN(("no path"));
			}
		}
	} else  {
		//LOG_DEBUG(("idle"));
		
	}
	
	calculateWayVelocity();
	
	if (!calculatingPath() && _velocity.is0()) {
		v3<float> dir = _target_position.convert<float>() - getPosition();
		dir.normalize();
		int t_dir = dir.getDirection(getDirectionsNumber()) - 1;
		if (t_dir != -1 && t_dir != getDirection())
			_velocity = dir;
	}
	updateStateFromVelocity();
}


