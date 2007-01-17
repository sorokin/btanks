#ifndef __BT_MAP_H__
#define __BT_MAP_H__

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


#include <map>
#include <string>
#include <stack>
#include "mrt/chunk.h"
#include "math/v3.h"
#include "math/matrix.h"
#include "mrt/singleton.h"

#include "notifying_xml_parser.h"

#include "sdlx/c_map.h"

namespace sdlx {
class Surface;
class Rect;
}

class TMXEntity;
class Layer;
class Object;

class IMap : public NotifyingXMLParser {
public:
	DECLARE_SINGLETON(IMap);
	
	typedef std::map<const std::string, std::string> PropertyMap;
	PropertyMap properties;

	IMap(); 
	~IMap();
	void clear();
	void load(const std::string &name);
	const std::string & getName() const { return _name; }
	const bool loaded() const;
	
	
	void render(sdlx::Surface &window, const sdlx::Rect &src, const sdlx::Rect &dst, const int z1, const int z2) const;
	const v3<int> getSize() const;
	const v3<int> getTileSize() const;
	
	virtual const int getImpassability(const Object *obj, const v3<int>& pos, v3<int> *tile_pos = NULL, bool *hidden = NULL) const;

	void getImpassabilityMatrix(Matrix<int> &matrix) const { matrix = _imp_map; }
	void getSurroundings(Matrix<int> &matrix, const v3<int> &pos, const int filler = -1) const;
	
	void damage(const v3<float> &position, const int hp);
	
	struct TileDescriptor {
		TileDescriptor() : surface(0), cmap(0), vmap(0) {}
		TileDescriptor(sdlx::Surface * surface, sdlx::CollisionMap *cmap, sdlx::CollisionMap *vmap) : 
			surface(surface), cmap(cmap), vmap(vmap) {}
		
		sdlx::Surface * surface;
		sdlx::CollisionMap *cmap, *vmap;
	};
	typedef std::vector< TileDescriptor > TileMap;
	
	void invalidateTile(const int xp, const int yp);

private:
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void charData(const std::string &data);

	Matrix<int> _imp_map;
	inline const bool collides(const Object *obj, const int dx, const int dy, const sdlx::CollisionMap *tile) const;
	inline const bool hiddenBy(const Object *obj, const int dx, const int dy, const sdlx::CollisionMap *tile) const;

	int _w, _h, _tw, _th, _firstgid;
	sdlx::CollisionMap _full_tile;
	
	int _lastz;
	mrt::Chunk _data;
	sdlx::Surface *_image;
	bool _image_is_tileset;

	PropertyMap _properties;
	
	typedef std::map<const int, Layer *> LayerMap;
	LayerMap _layers;
	Matrix<int> _cover_map;
	
	std::map<const std::string, std::string> _damage4;
	std::map<const std::string, int> _layer_z;
	bool _layer;
	std::string _layer_name;

	TileMap _tiles;

	struct Entity {
		mrt::XMLParser::Attrs attrs;
		std::string data;
		Entity(const mrt::XMLParser::Attrs & attrs) : attrs(attrs), data() {}
	};
	
	typedef std::stack<Entity> EntityStack;
	EntityStack _stack;
	
	std::string _name;
	
	IMap(const IMap&);
	const IMap& operator=(const IMap&);
};

SINGLETON(Map, IMap);

#endif

