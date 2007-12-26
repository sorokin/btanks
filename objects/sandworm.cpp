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
#include "world.h"
#include "registrar.h"
#include "math/matrix.h"
#include "ai/rush.h"
#include "tmx/map.h"
#include "mrt/random.h"
#include "config.h"

class SandWorm : public Object {
public: 
	SandWorm(): Object("monster"), _reaction_time(true), _fire(false), _head_id(0)  {
		setDirectionsNumber(1);
	}
	virtual void onSpawn() {
		disown();
		play("main", true);

		GET_CONFIG_VALUE("objects.sandworm.fire-rate", float, frt, 2.0f);
		_fire.set(frt);

		GET_CONFIG_VALUE("objects.sandworm.reaction-time", float, rt, 0.1);
		mrt::randomize(rt, rt/10);
		_reaction_time.set(rt);
		
		GET_CONFIG_VALUE("objects.sandworm.initial-length", int, il, 3);
		int i;
		for(i = 0; i < il; ++i) {
			if (_variants.has(mrt::formatString("%d", i))) {
				//LOG_DEBUG(("this is tail #%d", i));
				break;
			}
		}
		if (i < il) 
			speed = speed * 1.5;
		//LOG_DEBUG(("spawning tail #%d", i - 1));
		if (i > 0)
			spawn(mrt::formatString("sandworm(%d)", i - 1), "sandworm");
	}
	
	virtual void tick(const float dt) {
		if (_fire.tick(dt) && _state.fire && _head_id == 0) {
			GET_CONFIG_VALUE("objects.sandworm.minimum-snatch-distance", float, msd, 100.0f);
			v2<float> cpos; 
			getCenterPosition(cpos);
			
			if (!_variants.has("hunting")) {
				const Matrix<int> &hint = Map->getAreaMatrix("sandworm"); 
				const v2<int> tile_size = Map->getPathTileSize();
				v2<int> hint_pos = cpos.convert<int>() / tile_size;
				if (hint.get(hint_pos.y, hint_pos.x) == 0)
					return; //no snatchin' 
			}
			
		
			if (cpos.distance(_last_snatch) > msd) {
				std::set<const Object *> objects;
				GET_CONFIG_VALUE("objects.sandworm.snatch-range", float, sr, 32.0f);
				enumerateObjects(objects, sr, NULL);
				//LOG_DEBUG(("%u objects around", (unsigned) objects.size()));
				bool snatch = false;
				for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
					const Object *o = *i;
					if (o->speed != 0 && !o->piercing && o->registered_name != "mortar") {
						snatch = true;
						break;
					}
				}
				if (snatch) {
					Object *head = spawn("sandworm-head", "sandworm-head");
					_head_id = head->getID();
					_last_snatch = cpos;
				}
			}
		}
	}
	
	virtual void calculate(const float dt) {
		const bool active = _reaction_time.tick(dt);
		//head spawned
		if (_head_id) {
			_velocity.clear();
			if (!active)
				return;
			if (World->getObjectByID(_head_id) == NULL) {
				_head_id = 0;
				invalidate();
			}
		}
		
		int sid = getSummoner();
		_state.fire = false;

		if (active && sid <= 0) {
			_state.fire = true;
		}
		
		if (!active || isDriven()) {
			if (isDriven()) {
				calculateWayVelocity();
				updateStateFromVelocity();
			} 
			//else LOG_DEBUG(("tail <-> parent: velocity: %g %g", _velocity.x, _velocity.y));
			return;
		}

		if (sid > 0) {
			//tail following its predecessor.
			Object *summoner = World->getObjectByID(sid);
			if (summoner == NULL) {
				emit("death", NULL);
				return;
			}
			_velocity = getRelativePosition(summoner);
			float l = _velocity.normalize();
			if (l < (size.x  + size.y) / 2)
				_velocity.clear();
			//else _velocity.quantize8();
			return;
		}

		if (_variants.has("withdrawn")) {
			if (!isDriven()) {
				Way way;
				ai::Rush::calculateW(way, this, "sandworm");
				if (!way.empty()) 
					setWay(way);
			}
			return;
		}
		
		//LOG_DEBUG(("searching random hint area..."));
		const Matrix<int> &hint = Map->getAreaMatrix("sandworm"); 
		const v2<int> tile_size = Map->getPathTileSize();

		int w = hint.getWidth(), h = hint.getHeight();
		std::set<std::pair<int, int> > coords;
		for(int y = 0; y < h; ++y) 
			for(int x = 0; x < w; ++x) {
				if (hint.get(y, x))
					coords.insert(std::pair<int, int>(x, y));
		}
		
		if (coords.empty()) {
			LOG_ERROR(("no hint area defined... committing suicide"));
			emit("death", NULL);
			return;
		}
		
		int n = mrt::random(coords.size());
		std::set<std::pair<int, int> >::const_iterator i = coords.begin();
		while(n--) ++i;
		
		Way way;
		way.push_back(WayPoint(i->first * tile_size.x + tile_size.x / 2, i->second * tile_size.y + tile_size.y / 2));
		setWay(way);
	}
	
	Object* clone() const  {
		return new SandWorm(*this);
	}
	
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_reaction_time);
		s.add(_fire);
		s.add(_head_id);
		s.add(_last_snatch);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_reaction_time);
		s.get(_fire);
		s.get(_head_id);
		s.get(_last_snatch);
	}
	
private:
	Alarm _reaction_time, _fire; 
	int _head_id;
	v2<float> _last_snatch;
};

REGISTER_OBJECT("sandworm", SandWorm, ());


//sandworm head

class SandWormHead : public Object {
public:
	SandWormHead() : Object("monster") {}
	Object* clone() const  { return new SandWormHead(*this); }

	virtual void onSpawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);
};

void SandWormHead::onSpawn() {
	play("snatch", false);
}

void SandWormHead::tick(const float dt) {
	Object::tick(dt);

	if (getState().empty()) {
		Object::emit("death", NULL);
		return;
	};
}


void SandWormHead::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (emitter == NULL)
			return;
		if (emitter->piercing) {
			if (emitter->registered_name == "mortar-bullet")
				emit("death", emitter);
			return;
		}
		
		GET_CONFIG_VALUE("objects.sandworm-head.damage-after", float, da, 0.4);
		if (getStateProgress() < da)
			return;
		
		if (emitter == NULL || registered_name == "explosion" || 
			(emitter->registered_name.size() >= 9 && 
			emitter->registered_name.substr(emitter->registered_name.size() - 9, 9) == "explosion")
			|| emitter->speed == 0
			)
			return;
			
		emitter->Object::emit("death", this);
	} else if (event == "death") {
		//worm killed.
		int sid = getSummoner();
		Object * summoner = World->getObjectByID(sid);
		if (summoner)
			summoner->emit("death", this);
		Object::emit("death", emitter);
	} else 
		Object::emit(event, emitter);
}


REGISTER_OBJECT("sandworm-head", SandWormHead, ());

