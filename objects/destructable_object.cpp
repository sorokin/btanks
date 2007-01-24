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
#include "resource_manager.h"

DestructableObject::DestructableObject(const std::string &classname, const std::string &object, const std::string &animation, const bool make_pierceable) : 
		Object(classname), 
		_broken(false), _make_pierceable(make_pierceable),
		_object(object), _animation(animation) {}

void DestructableObject::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	s.add(_broken);
	s.add(_make_pierceable);
	s.add(_object);
	s.add(_animation);
}

void DestructableObject::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	s.get(_broken);
	s.get(_make_pierceable);
	s.get(_object);
	s.get(_animation);
}

void DestructableObject::addDamage(Object *from, const int dhp, const bool emitDeath) {
	if (_broken)
		return;

	Object::addDamage(from, dhp, false);
	if (hp <= 0) {
		_broken = true;
		if (_make_pierceable)
			pierceable = true;
		cancelAll();
		play("fade-out", false); 
		play("broken", true);
		
		if (!_object.empty() && !_animation.empty()) {
			v3<float> dpos; 
			dpos.z = getZ() + 1;
			spawn(_object, _animation, dpos, v3<float>::empty);
		}
	}
}

void DestructableObject::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
}

void DestructableObject::onSpawn() {
	play("main", true);
}


Object* DestructableObject::clone() const  {
	return new DestructableObject(*this);
}

REGISTER_OBJECT("destructable-object", DestructableObject, ("destructable-object", "", "", false));
REGISTER_OBJECT("destructable-object(pierceable)", DestructableObject, ("destructable-object", "", "", true));
REGISTER_OBJECT("destructable-object-with-fire", DestructableObject, ("destructable-object", "fire", "fire", false));
REGISTER_OBJECT("destructable-object-with-fire(pierceable)", DestructableObject, ("destructable-object", "fire", "fire", true));
