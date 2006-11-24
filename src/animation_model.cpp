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

#include "animation_model.h"
#include "mrt/logger.h"
#include "utils.h"

AnimationModel::AnimationModel(const float default_speed) : default_speed(default_speed) {}

void AnimationModel::addPose(const std::string &id, Pose *pose) {
	delete _poses[id];
	_poses[id] = pose;
	LOG_DEBUG(("pose '%s' with %d frames added (speed: %f)", id.c_str(), pose->frames.size(), pose->speed));
}

const Pose * AnimationModel::getPose(const std::string &id) const {
	PoseMap::const_iterator i = _poses.find(id);
	if (i == _poses.end())
		return NULL;
	return i->second;
}

AnimationModel::~AnimationModel() {
	std::for_each(_poses.begin(), _poses.end(), delete_ptr2<PoseMap::value_type>());
	_poses.clear();
}

Animation::Animation(const std::string & model, const std::string &surface, const int tw, const int th) : 
	model(model), surface(surface), tw(tw), th(th) {}
