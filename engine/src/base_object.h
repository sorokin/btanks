#ifndef __WORLD_BASE_OBJECT_H__
#define __WORLD_BASE_OBJECT_H__

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
#include <string>
#include <deque>
#include <set>

#include "export_btanks.h"
#include "math/v2.h"
#include "mrt/serializable.h" 
#include "player_state.h"
#include "object_common.h"
#include "variants.h"

namespace sdlx {
	class Surface;
}

namespace ai {
	class Buratino;
	class Waypoints;
}

class BTANKSAPI BaseObject : public mrt::Serializable {
public:
	v2<float> size;
	float mass, speed, ttl, impassability;
	int hp, max_hp;
	
	bool piercing, pierceable;
	
	std::string classname;
	
	bool disable_ai;
	
	BaseObject(const std::string &classname);
	virtual ~BaseObject();
	
	void updateVariants(const Variants &vars, const bool remove_old = false);
	
	virtual void tick(const float dt) = 0;
	virtual void render(sdlx::Surface &surf, const int x, const int y) = 0;
	void get_velocity(v2<float> &vel) const { vel = _velocity; vel.normalize(); vel *= speed; }
	static const float getCollisionTime(const v2<float> &pos, const v2<float> &vel, const float r);
	
	inline const bool isDead() const { return _dead; }
	inline const int getID() const { return _id; }

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	const std::string dump() const;
	void inheritParameters(const BaseObject *other);

	inline const PlayerState & getPlayerState() const { return _state; }
	const bool updatePlayerState(const PlayerState &state);

	void heal(const int hp);
	virtual const bool take(const BaseObject *obj, const std::string &type);
	
	void updateStateFromVelocity();
	void setZ(const int z, const bool absolute = false); 
	inline const int getZ() const { return _z; }
	
	void disown();
	inline const bool hasOwners() const { return !_owner_set.empty(); }
	void addOwner(const int oid);
	void prependOwner(const int oid); //to avoid truncation of this owner. 
	const int _getOwner() const;
	const bool hasOwner(const int oid) const;
	const bool hasSameOwner(const BaseObject *other, const bool skip_cooperative = false) const;
	void removeOwner(const int oid);
	void truncateOwners(const int n);
	inline void getOwners(std::deque<int> &owners) const { owners = _owners; }
	void copyOwners(const BaseObject *from);
	void copy_special_owners(const BaseObject *from);
	
	inline const int getSummoner() const { return _spawned_by; }
	
	void getTimes(float &moving, float &idle) const { moving = _moving_time; idle = _idle_time; }
	
	void interpolate();
	void uninterpolate();
	
	virtual void getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const;

	const float getEffectiveImpassability(const float impassability) const;
	
	const Variants & getVariants() const { return _variants; }
	
protected:
	int _id;
	PlayerState _state;
	v2<float> _velocity, _direction;
	float _moving_time, _idle_time;
	
	virtual void calculate(const float dt) = 0;

	bool _need_sync, _dead;
	Variants _variants;
	v2<float> _position;

private:
	
	//do not serialize interpolation stuff.
	v2<float> _interpolation_vector, _interpolation_position_backup;
	float _interpolation_progress;
	
	int _z;

	std::deque<int> _owners;
	std::set<int> _owner_set;

	int _spawned_by;
	
	friend class IWorld;
	friend class Object;
	friend class Teleport;
	friend class ai::Buratino;
	friend class ai::Waypoints;
};

#endif

