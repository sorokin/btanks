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

#include "map.h"
#include "layer.h"
#include "base_object.h"

#include "mrt/file.h"
#include "mrt/b64.h"
#include "mrt/gzip.h"
#include "mrt/logger.h"
#include "mrt/exception.h"

#include "sdlx/surface.h"
#include "sdlx/c_map.h"
#include "object.h"

#include <assert.h>
#include <limits>

#include "math/binary.h"

#include "config.h"

IMPLEMENT_SINGLETON(Map, IMap)

IMap::IMap() : _w(0), _h(0), _tw(0), _th(0), _ptw(0), _pth(0), _firstgid(0) {
	_lastz = -1000;
	_image = NULL;
}


inline const bool IMap::collides(const Object *obj, const int dx, const int dy, const sdlx::CollisionMap *tile) const {
	if (tile == NULL) {
		return false;
	}
	return obj->collides(tile, -dx, -dy);
}

inline const bool IMap::hiddenBy(const Object *obj, const int dx, const int dy, const sdlx::CollisionMap *tile) const {
	if (tile == NULL)
		return false;
	return obj->collides(tile, -dx, -dy, true);
}


const int IMap::getImpassability(const Object *obj, const v2<int>&pos, v2<int> *tile_pos, bool *hidden) const {
	if (obj->impassability <= 0) {
		return 0;
	}
	//LOG_DEBUG((">>IMap::getImpassability"));
	if (hidden)
		*hidden = false;

	GET_CONFIG_VALUE("engine.disable-outlines", bool, disable_outlines, false);

	if (disable_outlines) {
		hidden = NULL;
	}
	
	v2<float> position, velocity;
	obj->getInfo(position, velocity);
	
	const int obj_z = obj->getZ();
	int w = (int)obj->size.x, h = (int)obj->size.y;
	int x, x1;
	int y, y1;
	x = x1 = pos.x;
	y = y1 = pos.y;
	
	int x2 = x1 + w - 1; int y2 = y1 + h - 1;
	
	int xt1 = x1 / _tw; int xt2 = x2 / _tw;
	int yt1 = y1 / _th; int yt2 = y2 / _th; 
	int dx1 = x - xt1 * _tw; int dx2 = x - xt2 * _tw;
	int dy1 = y - yt1 * _th; int dy2 = y - yt2 * _th;
	
	int hidden_mask = 0;

	//LOG_DEBUG(("%d:%d:%d:%d (%+d:%+d:%+d:%+d)--> %d:%d %d:%d", x1, y1, w, h, dx1, dy1, dx2, dy2, xt1, yt1, xt2, yt2));
	int empty_mask = 0x0f;
	int im[4] = {101, 101, 101, 101};
	
	if (collides(obj, dx1, dy1, &_full_tile))
		empty_mask &= ~1;
	if (dy1 != dy2 && collides(obj, dx1, dy2, &_full_tile))
		empty_mask &= ~2;
	if (dx1 != dx2) {
		if (collides(obj, dx2, dy1, &_full_tile))
			empty_mask &= ~4;
		if (dy1 != dy2 && collides(obj, dx2, dy2, &_full_tile))
			empty_mask &= ~8;
	}
	
	for(LayerMap::const_reverse_iterator l = _layers.rbegin(); l != _layers.rend(); ++l) {
		const Layer *layer = l->second;
		int layer_im = layer->impassability;

		if (hidden && l->first > obj_z) {
			if (((hidden_mask & 1) == 0)) {
				if ((empty_mask & 1) || hiddenBy(obj, dx1, dy1, layer->getVisibilityMap(xt1, yt1)))
					hidden_mask |= 1;
			}
			
			if (yt1 != yt2 && ((hidden_mask & 2) == 0)) {
				if ((empty_mask & 2) || hiddenBy(obj, dx1, dy2, layer->getVisibilityMap(xt1, yt2)))
					hidden_mask |= 2;
			}
			
			if (xt1 != xt2) {
				if (((hidden_mask & 4) == 0)) {
					if ((empty_mask & 4) || hiddenBy(obj, dx2, dy1, layer->getVisibilityMap(xt2, yt1)))
						hidden_mask |= 4;
				}
				if (yt1 != yt2 && ((hidden_mask & 8) == 0)) {
					if ((empty_mask & 8) || hiddenBy(obj, dx2, dy2, layer->getVisibilityMap(xt2, yt2)))
						hidden_mask |= 8;
				}
			}
		}

		if (layer_im == -1 || (layer->pierceable && obj->piercing)) {
			continue;
		}

		//LOG_DEBUG(("im: %d, tile: %d", layer_im, layer->get(xt1, yt1)));
		
		if (!(empty_mask & 1) && im[0] == 101) {
			if (collides(obj, dx1, dy1, layer->getCollisionMap(xt1, yt1))) {
				im[0] = layer_im;
				if (yt1 == yt2 && im[1] == 101)
					im[1] = layer_im;
				if (xt1 == xt2) {
					if (im[2] == 101)
						im[2] = layer_im;
					if (yt1 == yt2 && im[3] == 101)
						im[3] = layer_im;
				}
			}
		}

		if (yt2 != yt1) {
			if (!(empty_mask & 2) && im[1] == 101) {
			if (collides(obj, dx1, dy2, layer->getCollisionMap(xt1, yt2))) {
				im[1] = layer_im;
				if (xt1 == xt2 && im[3] == 101) 
					im[3] = layer_im;
			}
			}
		}
		
		if (xt2 != xt1) {
			if (!(empty_mask & 4) && im[2] == 101) {
			if (collides(obj, dx2, dy1, layer->getCollisionMap(xt2, yt1))) {
				im[2] = layer_im;
				if (y1 == yt2 && im[3] == 101)
					im[3] = layer_im;

			}
			}
			if (yt2 != yt1 && im[3] == 101) { 
				if (!(empty_mask & 8)) {
				if (collides(obj, dx2, dy2, layer->getCollisionMap(xt2, yt2))) { 
					im[3] = layer_im;
				}
				}
			}
		}
	}

	int result_im = 0;
	//LOG_DEBUG(("im : %d %d", im[0], im[2])); 
	//LOG_DEBUG(("im : %d %d", im[1], im[3]));
	for(int i = 0; i < 4; ++i) if (im[i] == 101) im[i] = 0;
	
	if (tile_pos) {
		bool v1 = im[0] == 100 || im[1] == 100;
		bool v2 = im[2] == 100 || im[3] == 100;
		if (v1 && !v2) {
			tile_pos->x = tile_pos->x = _tw/2 + _tw * xt1;
		} else if (v2 && !v1) {
			tile_pos->x = tile_pos->x = _tw/2 + _tw * xt2;			
		} else 
			tile_pos->x = tile_pos->x = _tw/2 + _tw * (xt1 + xt2) / 2;
		
		bool h1 = im[0] == 100 || im[2] == 100;
		bool h2 = im[1] == 100 || im[3] == 100;
		if (h1 && !h2) {
			tile_pos->y = _th/2 + _th * yt1;
		} else if (h2 && !h1) {
			tile_pos->y = _th/2 + _th * yt2;
		} else 
			tile_pos->y = _th/2 + _th * (yt1 + yt2) / 2;
	}
	
	int t;
	if (velocity.y < 0 && (t = math::max(im[0], im[2])) < 101 ) {
		result_im = math::max(result_im, t);
		//LOG_DEBUG(("y<0 : %d", t));
	}
	if (velocity.y > 0 && (t = math::max(im[1], im[3])) < 101 ) {
		result_im = math::max(result_im, t);
		//LOG_DEBUG(("y>0 : %d", t));
	}
	if (velocity.x < 0 && (t = math::max(im[0], im[1])) < 101 ) {
		result_im = math::max(result_im, t);
		//LOG_DEBUG(("x<0 : %d", t));
	}
	if (velocity.x > 0 && (t = math::max(im[2], im[3])) < 101 ) {
		result_im = math::max(result_im, t);
		//LOG_DEBUG(("x>0 : %d", t));
	}
	
	if (xt1 == xt2) {
		hidden_mask |= 0x0c;
	}
	if (yt1 == yt2) {
		hidden_mask |= 0x0a;
	}
	
	if (hidden_mask == 0x0f && hidden)
		*hidden = true;

	assert(result_im >= 0 && result_im < 101);
	//LOG_DEBUG(("im = %d", result_im));
	//LOG_DEBUG(("<<IMap::getImpassability"));
	return result_im;
}


void IMap::load(const std::string &name) {
	clear();
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	
	LOG_DEBUG(("loading map '%s'", name.c_str()));
	const std::string file = data_dir + "/maps/" + name + ".tmx";
	parseFile(file);

	_name = name;
	
	_full_tile.create(_tw, _th, true);
	
	LOG_DEBUG(("optimizing layers..."));
	
	_cover_map.setSize(_h, _w, -10000);
	_cover_map.useDefault(-10000);
	
	unsigned int ot = 0;
	for(LayerMap::iterator l = _layers.begin(); l != _layers.end(); ++l) {
		l->second->optimize(_tiles);

		for(int ty = 0; ty < _h; ++ty) {
			for(int tx = 0; tx < _w; ++tx) {
				const sdlx::CollisionMap * vmap = l->second->getVisibilityMap(tx, ty);
				if (vmap == NULL)
					continue;
				if (vmap->isFull()) {
					_cover_map.set(ty, tx, l->first);
					++ot;
				}
			}
		}
	}
	LOG_DEBUG(("created render optimization map. opaque tiles found: %u", ot));
	//LOG_DEBUG(("rendering optimization map: %s", _cover_map.dump().c_str()));
	
	
	for(std::map<const std::string, std::string>::const_iterator i = _damage4.begin(); i != _damage4.end(); ++i) {
		Layer *dl = NULL, *l = NULL;
		dl = _layers[_layer_z[i->first]];
		if (dl == NULL)
			throw_ex(("layer %s doesnt exits", i->first.c_str()));
		l = _layers[_layer_z[i->second]];
		if (l == NULL)
			throw_ex(("layer %s doesnt exits", i->second.c_str()));
		LOG_DEBUG(("mapping damage layers: %s -> %s", i->first.c_str(), i->second.c_str()));
		ChainedDestructableLayer *cl = dynamic_cast<ChainedDestructableLayer *>(dl);
		if (cl == NULL) 
			throw_ex(("layer %s is not destructable", i->first.c_str()));
		cl->setSlave(l);
	}

#ifdef PRERENDER_LAYERS
	LOG_DEBUG(("rendering layers..."));
	for(LayerMap::iterator l = _layers.begin(); l != _layers.end(); ++l) {
		if (!l->visible)
			continue;
		
		l->second->surface.createRGB(_w * _tw, _h * _th, 24);
		//l->second->surface.convertAlpha();
		//l->second->surface.convertToHardware();
		
		for(int ty = 0; ty < _h; ++ty) {
			for(int tx = 0; tx < _w; ++tx) {
				const sdlx::Surface * s = l->second->getSurface(tx, ty);
				if (s == NULL) 
					continue;
				l->second->surface.copyFrom(*s, tx * _tw, ty * _th);
			}
		}
		//static int i;
		//l->second->surface.saveBMP(mrt::formatString("layer%d.bmp", i++));
	}
#endif
	
	GET_CONFIG_VALUE("map.pathfinding-step", int, ps, 32);
	
	const int split = 2 * ((_tw - 1) / 2 + 1) / ps;
	LOG_DEBUG(("split mode: %dx", split));
	
	_pth = _tw / split;
	_ptw = _th / split;
	_imp_map.setSize(_h * split, _w * split, -2);

	const int h = _imp_map.getHeight(), w = _imp_map.getWidth();
	LOG_DEBUG(("building map matrix[%d:%d]...", h, w));
	
	for(int y = 0; y < _h; ++y) {
		for(int x = 0; x < _w; ++x) {

			for(LayerMap::reverse_iterator l = _layers.rbegin(); l != _layers.rend(); ++l) {
				int im = l->second->impassability;
				if (im == -1)
					continue;
				
				int tid = l->second->get(x, y);
				if (tid == 0 || l->second->getCollisionMap(x, y)->isEmpty())
					continue;
				
				//break;
				//if (im == 100) 
				//	im = -1; //inf :)
				//_imp_map.set(y, x, im);
			
			
				Matrix<bool> proj;
				l->second->getCollisionMap(x, y)->project(proj, split, split);
				//LOG_DEBUG(("projection: %s", proj.dump().c_str()));
				//_imp_map.set(y, x, im);
				for(int yy = 0; yy < split; ++yy)
					for(int xx = 0; xx < split; ++xx) {
						int yp = y * split + yy, xp = x * split + xx;
						if (proj.get(yy, xx) && _imp_map.get(yp, xp) == -2) 
							_imp_map.set(yp, xp, im);
					}
				}
		}
	}
	for(int y = 0; y < h; ++y) 
		for(int x = 0; x < w; ++x) {
			if (_imp_map.get(y, x) == -2)
				_imp_map.set(y, x, 0);
			if (_imp_map.get(y, x) >= 100)
				_imp_map.set(y, x, -1);
	}
	_imp_map.useDefault(-1);
	LOG_DEBUG(("\n%s", _imp_map.dump().c_str()));
	
	LOG_DEBUG(("loading completed"));
}

void IMap::getSurroundings(Matrix<int> &matrix, const v2<int> &pos, const int filler) const {
	if (matrix.getWidth() % 2 == 0 || matrix.getHeight() % 2 == 0)
		throw_ex(("use only odd values for surrond matrix. (used: %d, %d)", matrix.getHeight(), matrix.getWidth()));
	
	int dx = (matrix.getWidth() - 1) / 2;
	int dy = (matrix.getHeight() - 1) / 2;
	
	v2<int> p = pos;
	p.x /= _tw;
	p.y /= _th;
	
	int y0 = p.y - dy, x0 = p.x - dx;
	for(int y = y0; y <= p.y + dy; ++y) 
		for(int x = x0; x <= p.x + dx; ++x) {
			int i = _imp_map.get(y, x);
			if (filler != -1 && i == -1)
				i = filler;
			matrix.set(y - y0, x - x0, i);
		}
}

void IMap::start(const std::string &name, Attrs &attrs) {
	//LOG_DEBUG(("started %s", name.c_str()));
	Entity e(attrs);
	
	if (name == "map") {
		LOG_DEBUG(("map file version %s", e.attrs["version"].c_str()));
		_w = atol(e.attrs["width"].c_str());
		_h = atol(e.attrs["height"].c_str());
		_tw = atol(e.attrs["tilewidth"].c_str());
		_th = atol(e.attrs["tileheight"].c_str());
		
		if (_tw < 1 || _th < 1 || _w < 1 || _h < 1)
			throw_ex(("invalid map parameters. %dx%d tile: %dx%d", _w, _h, _tw, _th));
		
		LOG_DEBUG(("initializing map. size: %dx%d, tilesize: %dx%d", _w, _h, _tw, _th));
	} else if (name == "tileset") {
		_firstgid = atol(e.attrs["firstgid"].c_str());
		if (_firstgid < 1) 
			throw_ex(("tileset.firstgid must be > 0"));
		LOG_DEBUG(("tileset: '%s'. firstgid = %d", e.attrs["name"].c_str(), _firstgid));
	} else if (name == "layer") {
		_properties.clear();
		_layer = true;
		_layer_name = e.attrs["name"];
		if (_layer_name.empty())
			throw_ex(("layer name cannot be empty!"));
	}
	
	_stack.push(e);
	NotifyingXMLParser::start(name, attrs);
}

void IMap::end(const std::string &name) {
	assert(!_stack.empty());
	Entity &e = _stack.top();
	
	if (name == "tile") {
		if (e.attrs.find("id") == e.attrs.end())
			throw_ex(("tile.id was not found")); 
			
		if (_image == NULL) 
			throw_ex(("tile must contain <image> inside it."));
		
		unsigned int id = atol(e.attrs["id"].c_str());
		id += _firstgid;
		LOG_DEBUG(("tile gid = %d, image: %p", id, (void *)_image));

		//TileManager->set(id, _image);
		//_tiles.reserve(id + 2);
		if (id >= _tiles.size())
			_tiles.resize(id + 20);
		
		TileMap::value_type &tile = _tiles[id];	
		if (tile.surface != NULL)
			throw_ex(("duplicate tile %d found", id));

		tile.cmap = new sdlx::CollisionMap;
		tile.cmap->init(_image, sdlx::CollisionMap::OnlyOpaque);
		tile.vmap = new sdlx::CollisionMap;
		tile.vmap->init(_image, sdlx::CollisionMap::AnyVisible);
		tile.surface = _image;
		
		_image = NULL;

	} else if (name == "data") {
		std::string enc = e.attrs["encoding"];
		if (enc.size() == 0) enc = "none";
		std::string comp = e.attrs["compression"];
		if (comp.size() == 0) comp = "none";

		LOG_DEBUG(("data found. encoding: %s, compression: %s", enc.c_str(), comp.c_str()));
		
		mrt::Chunk data;
		if (enc == "base64") {
			mrt::Base64::decode(data, e.data);
		} else if (enc == "none") {
			data.setData(e.data.c_str(), e.data.size());
		} else throw_ex(("unknown encoding %s used", enc.c_str()));
		
		//LOG_DEBUG(("decoded size: %d", data.getSize()));
		//LOG_DEBUG(("decoded data: %s -> %s", e.data.c_str(), data.dump().c_str()));

		if (comp == "gzip") {
			mrt::ZStream::decompress(_data, data);
		} else if (comp == "none") {
			_data = data;
		} else throw_ex(("unknown compression method ('%s') used. ", comp.c_str()));
		data.free();
		//LOG_DEBUG(("%s", _data.dump().c_str()));
	} else if (name == "image") {
		delete _image;
		_image = NULL;
		
		_image = new sdlx::Surface;
		std::string source = e.attrs["source"];
		GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");

		if (source.size()) {
			source = data_dir + "/tiles/" + source;
			LOG_DEBUG(("loading tileset from single file ('%s')", source.c_str()));
			_image->loadImage(source);
			_image_is_tileset = true;
		} else {
			_image->loadImage(_data);
			_image_is_tileset = false;
		}
		//_image->convert(SDL_ASYNCBLIT | SDL_HWSURFACE);
		_image->convertAlpha();
		_image->convertToHardware();
		
		LOG_DEBUG(("image loaded. (%dx%d) format: %s", _image->getWidth(), _image->getHeight(), e.attrs["format"].c_str()));
	} else if (name == "layer") {
		int w = atol(e.attrs["width"].c_str());
		int h = atol(e.attrs["height"].c_str());
		int z = (_properties.find("z") == _properties.end())?++_lastz:atol(_properties["z"].c_str());
		_lastz = z;
		int impassability = (_properties.find("impassability") != _properties.end())?atoi(_properties["impassability"].c_str()):-1;
		
		bool pierceable = false;
		
		int hp = atoi(_properties["hp"].c_str());
		
		PropertyMap::const_iterator pi = _properties.find("pierceable");
		if (pi != _properties.end()) {
			pierceable = true;
			if (!pi->second.empty()) {
				unsigned char pc = pi->second[0];
				pierceable = pc == 't' || pc == 'T' || pc == '1';
			}
		}
		Layer *layer = NULL;
		if (!_properties["visible-if-damaged"].empty()) {
			layer = new DestructableLayer(true);
		}
		if (!_properties["invisible-if-damaged"].empty()) {
			if (layer != NULL) 
				throw_ex(("visible/invisible options is mutually exclusive"));
			layer = new DestructableLayer(false);
		}
		const std::string damage = _properties["damage-for"];
		if (!damage.empty()) {
			if (layer != NULL)
				throw_ex(("damage-for cannot be combined with (in)visible-if-damaged"));
			layer = new ChainedDestructableLayer();
			_damage4[_layer_name] = damage;
		}
		LOG_DEBUG(("layer '%s'. %dx%d. z: %d, size: %d, impassability: %d", e.attrs["name"].c_str(), w, h, z, _data.getSize(), impassability));
		if (_layers.find(z) != _layers.end())
			throw_ex(("layer with z %d already exists", z));
		if(layer == NULL)
			layer = new Layer;
		
		layer->impassability = impassability;
		layer->pierceable = pierceable;
		layer->hp = hp;
		TRY { 
			layer->init(w, h, _data); //fixme: fix possible memory leak here, if exception occurs
		} CATCH(mrt::formatString("layer '%s'", _layer_name.c_str()).c_str(), throw);
		
		_layers[z] = layer;
		_layer_z[_layer_name] = z;
		//LOG_DEBUG(("(1,1) = %d", _layers[z]->get(1,1)));
		_layer = false;
	} else if (name == "property") {
		mrt::trim(e.attrs["name"]);
		if (_layer)
			_properties[e.attrs["name"]] = e.attrs["value"];
		else 
			properties[e.attrs["name"]] = e.attrs["value"];
	} else if (name == "tileset" && _image != NULL && _image_is_tileset) {
		//fixme: do not actualy chop image in many tiles at once, use `tile' wrapper
		_image->setAlpha(0, 0);
		int w = _image->getWidth(), h = _image->getHeight();
		int id = 0;
		for(int y = 0; y < h; y += _th) {
			for(int x = 0; x < w; x += _tw) {
				sdlx::Surface *s = new sdlx::Surface;
				s->createRGB(_tw, _th, 24);
				s->convertAlpha();
				s->convertToHardware();

				sdlx::Rect from(x, y, _tw, _th);
				s->copyFrom(*_image, from);
				//s->saveBMP(mrt::formatString("tile-%d.bmp", id));

				//LOG_DEBUG(("cut tile %d from tileset [%d:%d, %d:%d]", _firstgid + id, x, y, _tw, _th));
				if ((size_t)(_firstgid + id) >= _tiles.size())
					_tiles.resize(_firstgid + id + 20);
				
				delete _tiles[_firstgid + id].surface;
				_tiles[_firstgid + id].surface = NULL;
				delete _tiles[_firstgid + id].cmap;
				_tiles[_firstgid + id].cmap = NULL;
				delete _tiles[_firstgid + id].vmap;
				_tiles[_firstgid + id].vmap = NULL;
				
				_tiles[_firstgid + id].cmap = new sdlx::CollisionMap;
				_tiles[_firstgid + id].cmap->init(s, sdlx::CollisionMap::OnlyOpaque);
				_tiles[_firstgid + id].vmap = new sdlx::CollisionMap;
				_tiles[_firstgid + id].vmap->init(s, sdlx::CollisionMap::AnyVisible);
				_tiles[_firstgid + id].surface = s;
				++id;
				s = NULL;
			}
		}

		delete _image;
		_image = NULL;
	}
	
	_stack.pop();
	NotifyingXMLParser::end(name);
}

void IMap::charData(const std::string &d) {
	assert(!_stack.empty());
	//LOG_DEBUG(("char1 %s", d.c_str()));
	std::string data(d);
	mrt::trim(data);
	if (data.size() == 0)
		return;
	
	//LOG_DEBUG(("char2 %s", data.c_str()));
	_stack.top().data += d;
}

void IMap::render(sdlx::Surface &window, const sdlx::Rect &src, const sdlx::Rect &dst, const int z1, const int z2) const {
	if (_w == 0 || z1 >= z2)  //not loaded
		return;
#ifdef PRERENDER_LAYERS
	for(LayerMap::const_iterator l = _layers.begin(); l != _layers.end(); ++l) 	
		
		if (l->first >= z1) {
			if (l->first >= z2) 
				break;
			window.copyFrom(l->second->surface, src);
		}
#else
	int txp = src.x / _tw, typ = src.y / _th;
	int xp = - (src.x % _tw), yp = -(src.y % _th);
	
	int txn = (src.w - 1) / _tw + 2;
	int tyn = (src.h - 1) / _th + 2;
	
	//unsigned int skipped = 0;
	
	for(LayerMap::const_iterator l = _layers.begin(); l != _layers.end(); ++l) {
		const int z = l->first;
		if (z < z1) 
			continue;
		
		if (z >= z2) 
			break;

		//LOG_DEBUG(("z: %d << %d, layer: %d", z1, z2, l->first));
		
		for(int ty = 0; ty < tyn; ++ty) {
			for(int tx = 0; tx < txn; ++tx) {
				if (z < _cover_map.get(typ + ty, txp + tx)) {//this tile covered by another tile
					//++skipped;
					continue;
				}
				
				const sdlx::Surface * s = l->second->getSurface(txp + tx, typ + ty);
				if (s != NULL) 
					window.copyFrom(*s, dst.x + xp + tx * _tw, dst.y + yp + ty * _th);
			}
		}
	}
	//LOG_DEBUG(("blits skipped: %u", skipped));
	//LOG_DEBUG(("====================================="));
#endif
}


void IMap::clear() {
	for(LayerMap::iterator i = _layers.begin(); i != _layers.end(); ++i) {
		delete i->second;
	}
	_layers.clear();
	
	for(TileMap::iterator i = _tiles.begin(); i != _tiles.end(); ++i) {
		delete i->surface;
		delete i->cmap;
		delete i->vmap;
	}
	_tiles.clear();
	
	properties.clear();
	_properties.clear();

	_image = NULL;
	_lastz = -100;
	_w = _h = _tw = _th = _firstgid = 0;

	_damage4.clear();
	_layer_z.clear();
	_cover_map.setSize(0, 0, 0);
}

IMap::~IMap() {
	LOG_DEBUG(("cleaning up map..."));
	clear();
}

const bool IMap::loaded() const {
	return _w != 0;
}

const v2<int> IMap::getSize() const {
	return v2<int>(_tw * _w,_th * _h);
}

const v2<int> IMap::getTileSize() const {
	return v2<int>(_tw, _th);
}

const v2<int> IMap::getPathTileSize() const {
	return v2<int>(_ptw, _pth);
}


void IMap::damage(const v2<float> &position, const int hp) {
	v2<int> pos = position.convert<int>();
	pos.x /= _tw;
	pos.y /= _th;
	//LOG_DEBUG(("map damage: %g:%g -> %d:%d for %d hp", position.x, position.y, pos.x, pos.y, hp));
	for(LayerMap::iterator i = _layers.begin(); i != _layers.end(); ++i) {
		i->second->damage(pos.x, pos.y, hp);
	}
}

void IMap::invalidateTile(const int xp, const int yp) {
	_cover_map.set(yp, xp, -10000);
}
