#ifndef __BT_LAYER_H__
#define __BT_LAYER_H__

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


#include "mrt/chunk.h"
#include "sdlx/surface.h"
#include "sdlx/c_map.h"
#include <vector>
#include "tmx/map.h"

#define PRERENDER_LAYERS
#undef PRERENDER_LAYERS

#if defined(__GNUC__)
#	define restrict __restrict__
#elif !defined(restrict)
#	define restrict
#endif

class Layer {
public:
#ifdef PRERENDER_LAYERS
	sdlx::Surface surface;
#endif
	int impassability, hp;
	bool pierceable;
	bool animated;

	Layer();
	virtual void init(const int w, const int h, const mrt::Chunk & data);
	void setAnimation(const int frame_size, const float speed);
	virtual void tick(const float dt);

	void clear(const int idx);
	
	virtual const Uint32 get(const int idx) const; 
	virtual const Uint32 get(const int x, const int y) const; 
	
	virtual const bool damage(const int x, const int y, const int hp);
	virtual void _destroy(const int x, const int y);

	virtual ~Layer();

protected: 
	int _w, _h;
private: 
	float pos, speed;
	int base, frame_size;
	mrt::Chunk _data; //hands off, you stupid layers! :)
};

class DestructableLayer : public Layer {
public: 
	DestructableLayer(const bool visible_if_damaged);
	virtual void init(const int w, const int h, const mrt::Chunk & data);

	virtual const Uint32 get(const int x, const int y) const; 

	virtual const bool damage(const int x, const int y, const int hp);
	virtual void _destroy(const int x, const int y);
	virtual void onDeath(const int idx);
	
	~DestructableLayer();
protected:
	int* restrict _hp_data;
	bool _visible;
};

class ChainedDestructableLayer : public DestructableLayer {
public: 
	ChainedDestructableLayer() : DestructableLayer(true), _slave(NULL) {}
	void setSlave(Layer *layer) { _slave = layer; }

	virtual void onDeath(const int idx);
private: 
	Layer *_slave;
};

#endif

