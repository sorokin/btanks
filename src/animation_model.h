#ifndef __BTANKS_ANIMATION_MODEL_H__
#define __BTANKS_ANIMATION_MODEL_H__

/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include <vector>
#include <string>
#include <map>

class Pose {
public:
	Pose(const float speed, const float z, const std::string &sound) : speed(speed), z(z), sound(sound) {}

	const float speed, z;
	std::string sound;
	std::vector<unsigned int> frames;
};


class AnimationModel {
public:
	const float default_speed;
	AnimationModel(const float default_speed);
	
	void addPose(const std::string &id, Pose *pose);
	const Pose * getPose(const std::string &id) const;
	~AnimationModel();

private:
	typedef std::map<const std::string, Pose *> PoseMap;
	PoseMap _poses;
};

#endif
