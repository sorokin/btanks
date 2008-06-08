
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
#include "heli.h"
#include "config.h"
#include "registrar.h"
#include "tmx/map.h"
#include "mrt/random.h"
#include "ai/base.h"
#include "ai/targets.h"

class AIHeli : public Heli, public ai::Base {
public:
	AIHeli() : Heli("helicopter"), _reaction(true), _target_dir(-1) {
	}
	virtual void onSpawn();
	void calculate(const float dt);
	virtual void serialize(mrt::Serializator &s) const {
		Heli::serialize(s);
		ai::Base::serialize(s);
		s.add(_reaction);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Heli::deserialize(s);
		ai::Base::deserialize(s);
		s.get(_reaction);
	}
	
	virtual const bool validateFire(const int idx) {
		return (idx == 0)? canFire() : true;
	}

	virtual Object * clone() const { return new AIHeli(*this); }
	virtual void onIdle(const float dt);
	
private: 
	Alarm _reaction;
	int _target_dir;
};

void AIHeli::onIdle(const float dt) {
	Way way;
	v2<int> map_size = Map->get_size();
	
	for(int i = 0; i < 2; ++i) {
		v2<int> next_target;
		next_target.x = (int)size.x / 2 + mrt::random(map_size.x - (int)size.x);
		next_target.y = (int)size.y / 2 + mrt::random(map_size.y - (int)size.y);
		way.push_back(next_target);		
	}
	setWay(way);
}


void AIHeli::onSpawn() {
	GET_CONFIG_VALUE("objects.helicopter.reaction-time", float, rt, 0.1);
	mrt::randomize(rt, rt/10);
	_reaction.set(rt);
	Heli::onSpawn();
	ai::Base::onSpawn(this);
	ai::Base::multiplier = 3.0f;
}

void AIHeli::calculate(const float dt) {
	v2<float> vel;
	if (!_reaction.tick(dt))
		goto done;
		
	_state.fire = false;
	
	_target_dir = getTarget_position(_velocity, ai::Targets->troops, "helicopter-bullet");
	if (_target_dir >= 0) {
		//LOG_DEBUG(("target: %g %g %g, dir: %d", _velocity.x, _velocity.y, _velocity.length(), _target_dir));
		/*
		Way way;
		if (findPath(tp, way)) {
		setWay(way);
			calculateWayVelocity();
		}
		*/
		if (_velocity.length() >= 25) {
			quantizeVelocity();
			//_direction.fromDirection(get_direction(), get_directions_number());
		} else {
			_velocity.clear();
			setDirection(_target_dir);
			//LOG_DEBUG(("%d", _target_dir));
			_direction.fromDirection(_target_dir, get_directions_number());
		}

		if (_target_dir == get_direction()) {
			_state.fire = true;
		}	
	} 
	
	if (_target_dir < 0 && !isDriven()) {
		_velocity.clear();
		_target_dir = -1;
		onIdle(dt);
	}
	
done: 	
	GET_CONFIG_VALUE("engine.mass-acceleration-divisor", float, ac_div, 1000.0f);

	const float ac_t = mass / ac_div * 0.8;
	_state.alt_fire = _moving_time >= ac_t;

	calculateWayVelocity();

	GET_CONFIG_VALUE("objects.helicopter.rotation-time", float, rt, 0.2f);
	limitRotation(dt, rt, true, true);	
	updateStateFromVelocity();
}

REGISTER_OBJECT("helicopter", AIHeli, ());
