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

class DestructableObject : public Object {
public:
	DestructableObject(const std::string &classname, const std::string &object, const std::string &animation, const bool make_pierceable);

	virtual Object * clone() const;
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual void addDamage(Object *from, const int hp, const bool emitDeath = true);

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

protected:
	bool _broken;

private: 
	bool _make_pierceable;
	std::string _object, _animation;
};
