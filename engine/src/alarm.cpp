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

#include "alarm.h"
#include "mrt/serializator.h"
#include <assert.h>

Alarm::Alarm(const float period, const bool repeat) : _period(period), _t(0), _repeat(repeat) {}
Alarm::Alarm(const bool repeat): _period(0), _t(0), _repeat(repeat) {}

const bool Alarm::tick(const float dt) {
	assert(_period != 0);
	if (dt < 0) {
		return false;
	}

	_t += dt;
	if (_t < _period)
		return false;

	if (_period) {
		while(_t > _period)
			_t -= _period;
	}
	
	return true;
}

const float Alarm::get() const {
	return (_t >= _period)?1: _t / _period;
}

void Alarm::set(const float period, const bool reset) {
	_period = period;
	if (reset)
		_t = 0;
}

void Alarm::reset() {
	_t = 0;
}

void Alarm::serialize(mrt::Serializator &s) const {
	s.add(_period);
	s.add(_t);
	s.add(_repeat);
}

void Alarm::deserialize(const mrt::Serializator &s) {
	s.get(_period);
	s.get(_t);
	s.get(_repeat);
}
