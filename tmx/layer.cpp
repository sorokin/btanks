
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

#include "layer.h"
#include <assert.h>
#include "mrt/exception.h"
#include <assert.h>
#include <queue>
#include <set>

#include "world.h"
#include "map.h"
#include "object.h"
#include "resource_manager.h"
#include "animation_model.h"
#include "mrt/random.h"

void ChainedDestructableLayer::onDeath(const int idx) {
	DestructableLayer::onDeath(idx);
	_slave->clear(idx);
}

void DestructableLayer::onDeath(const int idx) {
	_hp_data[idx] = -1;
	const int x = idx % _w, y = idx / _w;
	Map->invalidateTile(x, y);
	
	const sdlx::Surface *s = NULL;
	const sdlx::CollisionMap *cm = NULL;
	ResourceManager->checkSurface("building-explosion", s, cm);
	assert(s != NULL);
	
	Object * o = ResourceManager->createObject("explosion", "building-explosion");
	v2<int> tsize = Map->getTileSize();
	v2<float> pos(x * tsize.x + tsize.x/2, y * tsize.y + tsize.y/2); //big fixme.
	pos -= o->size / 2;
	
	int dirs = (s->getWidth() - 1) / (int)o->size.x + 1;
	int dir = mrt::random(dirs);
	//LOG_DEBUG(("set dir %d (%d)", dir, dirs));
	o->setDirectionsNumber(dirs);
	o->setDirection(dir);
	
	World->addObject(o, pos);
}

DestructableLayer::DestructableLayer(const bool visible) : _hp_data(NULL), _visible(visible) {}

void DestructableLayer::init(const int w, const int h, const mrt::Chunk & data) {
	if (hp <= 0)
		throw_ex(("destructable layer cannot have hp %d (hp must be > 0)", hp));
	Layer::init(w, h, data);
	
	int size = _w * _h;
	delete[] _hp_data;
	_hp_data = new int[size];

	for(int i = 0; i < size; ++i) {
		_hp_data[i] = (Layer::get(i) != 0) ? hp : 0;
	}
}
const Uint32 DestructableLayer::get(const int x, const int y) const {
	int i = _w * y + x;
	if (i < 0 || i >= _w * _h)
		return 0;
	const bool visible = _visible ? (_hp_data[i] == -1) : (_hp_data[i] > 0);
	return visible? Layer::get(i): 0;
}

const bool DestructableLayer::damage(const int x, const int y, const int hp) {
	const int i = _w * y + x;
	if (i < 0 || i >= _w * _h)
		return false;
	//LOG_DEBUG(("damage %d to cell %d", hp, i));
	if (_hp_data[i] <= 0) 
		return false;
	
	_hp_data[i] -= hp;
	if (_hp_data[i] > 0)
		return false;
	
	_destroy(x, y);
	return true;
}

void DestructableLayer::_destroy(const int x, const int y) {
	//_hp_data[i] = -1; //destructed cell
	const int i = _w * y + x;
	const int size = _w * _h;
	
	std::queue<int> queue;
	std::set<int> visited;
	queue.push(i);
	while(!queue.empty()) {
		int v = queue.front();
		queue.pop();
		
		assert( v >= 0 && v < size );
		if (visited.find(v) != visited.end())
			continue;
		
		visited.insert(v);
		
		int x = v % _w, y = v / _w;
		//LOG_DEBUG(("checking %d %d -> %d", x, y, get(x, y)));
		if (Layer::get(x, y) == 0)
			continue;
		
		onDeath(v);
		
		if (x > 0)
			queue.push(v - 1);
		if (x < _w - 1)
			queue.push(v + 1);
		if (y > 0)
			queue.push(v - _w);
		if (y < _h - 1)
			queue.push(v + _w);
	}
	//LOG_DEBUG(("cleanup done"));
}

DestructableLayer::~DestructableLayer() {
	delete[] _hp_data;
}

Layer::Layer() : impassability(0), hp(0), pierceable(false), _w(0), _h(0), pos(0), speed(1), base(0), frames(0), frame_size(0) {}

void Layer::setAnimation(const int frame_size, const int frames, const float speed) {
	if (frame_size < 1) 
		throw_ex(("animation frame size %d is invalid", frame_size));
	if (frames < 1) 
		throw_ex(("animation frames number %d is invalid", frames));
	if (speed <= 0)
		throw_ex(("animation speed %g is invalid", speed));
	this->frame_size = frame_size;
	this->frames = frames;
	this->speed = speed;
}

void Layer::tick(const float dt) {
	if (frames == 0 || frame_size == 0)
		return;
	pos += speed * dt;
	int p = (int)(pos / frames);
	pos -= p * frames;
	int f = (int)pos;
	f %= frames;

	base = f * frame_size;
	//LOG_DEBUG(("pos : %g, n: %d, frame: %d -> base: %d", pos, frames, f, base));
}

void Layer::init(const int w, const int h, const mrt::Chunk & data) {
	_w = w;
	_h = h;
	_data = data;
	assert((int)_data.getSize() == (4 * _w * _h));
}


const Uint32 Layer::get(const int idx) const {
	const int i = idx + base; //haha!
	if (i < 0 || i / 4 >= (int)_data.getSize())
		return 0;
	return *((Uint32 *) _data.getPtr() + i);
}


const Uint32 Layer::get(const int x, const int y) const {
	return get(_w * y + x);
}

void Layer::clear(const int i) {
	if (i < 0 || i >= _w * _h)
		return;
	*((Uint32 *) _data.getPtr() + i) = 0;
}

const bool Layer::damage(const int x, const int y, const int hp) { return false; }
void Layer::_destroy(const int x, const int y) {}


Layer::~Layer() { }
