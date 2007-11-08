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

#include "tank.h"
#include "mrt/exception.h"
#include "registrar.h"
#include "config.h"

#include "ai/buratino.h" 

class AITank:  public Tank, public ai::Buratino {
public: 
	AITank(const std::string &classname) : Tank(classname) {}
//	~AITank();
	virtual void onSpawn();
	virtual void calculate(const float dt);

	virtual Object * clone() const { return new AITank(*this); }

	virtual const std::string getWeapon(const int idx) const;
	virtual const int getWeaponAmount(const int idx) const;
private: 

};

const std::string AITank::getWeapon(const int idx) const {
	switch(idx) {
	case 0: 
		if (isEffectActive("dirt")) {
			return "bullets:dirt";
		} else if (isEffectActive("dispersion")) {
			return "bullets:dispersion";
		} else if (isEffectActive("ricochet")) {
			return "bullets:ricochet";
		}
		return "bullet";
	case 1:
		return get( "mod")->getType();
	default: 
		throw_ex(("weapon %d doesnt supported", idx));
	}
}

const int AITank::getWeaponAmount(const int idx) const{
	switch(idx) {
	case 0: 
		return -1;
	case 1:
		return get("mod")->getCount();
	default: 
		throw_ex(("weapon %d doesnt supported", idx));
	}
}


void AITank::onSpawn() {
	addEnemyClass("fighting-vehicle");
	addEnemyClass("trooper");
	addEnemyClass("kamikaze");
	addEnemyClass("boat");
	addEnemyClass("helicopter");
	addEnemyClass("watchtower");
	addEnemyClass("barrack");
	addEnemyClass("monster");
	
	addBonusName("heal");
	addBonusName("megaheal");
	
	addBonusName("teleport");

	//primary weapon	
	addBonusName("dispersion-bullets-item");
	addBonusName("ricochet-bullets-item");

	//secondary weapon
	addBonusName("guided-missiles-item");
	addBonusName("dumb-missiles-item");
	addBonusName("nuke-missiles-item");
	addBonusName("boomerang-missiles-item");
	addBonusName("stun-missiles-item");
	addBonusName("mines-item");

	ai::Buratino::onSpawn(this);
	Tank::onSpawn();
}

void AITank::calculate(const float dt) {
	ai::Buratino::calculate(this, dt);
	
	GET_CONFIG_VALUE("objects.tank.rotation-time", float, rt, 0.05);
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();	
}

REGISTER_OBJECT("tank", AITank, ("fighting-vehicle"));
REGISTER_OBJECT("static-tank", AITank, ("vehicle"));
