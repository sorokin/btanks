
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
#include "alarm.h"
#include "tmx/map.h"
#include "mrt/random.h"
#include "mrt/logger.h"
#include "resource_manager.h"
#include "config.h"
#include "world.h"

class Helicopter : public Object {
public:
	Helicopter(const std::string &para) :
		 Object("helicopter"), 
		 _next_target(), _next_target_rel(),
		 _active(false), _spawn(true), _paratrooper(para) {}
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual Object * clone() const;
	void onSpawn();

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		_next_target.serialize(s);
		_next_target_rel.serialize(s);
		s.add(_active);
		_spawn.serialize(s);
		s.add(_paratrooper);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_next_target.deserialize(s);
		_next_target_rel.deserialize(s);
		s.get(_active);
		_spawn.deserialize(s);
		s.get(_paratrooper);
	}

private: 
	v3<float> _next_target, _next_target_rel;
	bool _active;
	Alarm _spawn;
	std::string _paratrooper;
};

void Helicopter::onSpawn() {
	play("main", true);
	GET_CONFIG_VALUE("objects.helicopter-with-kamikazes.spawn-rate", float, sr, 1.5);
	_spawn.set(sr);
}

void Helicopter::tick(const float dt) {
	Object::tick(dt);
	GET_CONFIG_VALUE("objects.helicopter-with-kamikazes.maximum-children", int, max_c, 10);
	if (_active && _spawn.tick(dt)) {
		if (World->getChildren(getID()) >= max_c) 
			return;
		
		Matrix<int> matrix; 
		World->getImpassabilityMatrix(matrix, this, NULL);

		v3<int> pos, pos2;
		getCenterPosition(pos); 
		v3<int> para_size = ResourceManager->getClass(_paratrooper)->size.convert<int>();
		pos -= para_size / 2;
		
		pos2 = pos;
		pos2 += para_size;
		pos2 -= 1;

		const v3<int> tile_size = Map->getTileSize();

		pos /= tile_size;
		pos2 /= tile_size;
		/*
		LOG_DEBUG(("%d %d", matrix.get(pos.y, pos.x), matrix.get(pos.y, pos2.x)));
		LOG_DEBUG(("%d %d", matrix.get(pos2.y, pos.x), matrix.get(pos2.y, pos2.x)));
		*/
		if (matrix.get(pos.y, pos.x) == -1 || matrix.get(pos.y, pos2.x) == -1 || 
			matrix.get(pos2.y, pos.x) == -1 || matrix.get(pos2.y, pos2.x) == -1) {
				LOG_DEBUG(("cannot drop paratrooper, sir!"));
			} else {
				int pt = mrt::random(6);
				//LOG_DEBUG(("ptype = %d", pt));
				std::string animation = (pt == 3)? "gay-paratrooper": "paratrooper";
					
				spawn(_paratrooper, animation);
			}
	} 
	if (!_active)
		_velocity.clear();
}


void Helicopter::calculate(const float dt) {
	GET_CONFIG_VALUE("objects.helicopter-with-kamikazes.delay-before-next-target", float, delay, 1.0);
	v3<float> pos = getPosition();

	if (!_active && _idle_time > delay) { 

		v3<int> size = Map->getSize();
		_next_target.x = mrt::random(size.x);
		_next_target.y = mrt::random(size.y);
		_next_target_rel = _next_target - pos;
		LOG_DEBUG(("picking up random target: %g %g", _next_target.x, _next_target.x));
		_active = true;
	}
	if (_active) {
		_velocity = _next_target - pos;
		//LOG_DEBUG(("vel: %g %g, rel: %g %g", _velocity.x, _velocity.y, _next_target_rel.x, _next_target_rel.y));
		if (_velocity.is0() || (_velocity.x * _next_target_rel.x) < 0 || (_velocity.y * _next_target_rel.y) < 0 ) {
			_active = false; 
			LOG_DEBUG(("stop"));
			_velocity.clear();
		} 
	} else _velocity.clear();
	//LOG_DEBUG(("vel: %g %g", _velocity.x, _velocity.y));
	
	GET_CONFIG_VALUE("objects.helicopter.rotation-time", float, rt, 0.2);
	limitRotation(dt, rt, true, false);
}


Object* Helicopter::clone() const  {
	return new Helicopter(*this);
}

REGISTER_OBJECT("helicopter-with-kamikazes", Helicopter, ("paratrooper-kamikaze"));
