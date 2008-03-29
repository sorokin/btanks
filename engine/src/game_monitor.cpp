
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
#include <stdexcept>
#include <stdlib.h>

#include "game_monitor.h"
#include "object.h"
#include "config.h"
#include "world.h"
#include "resource_manager.h"
#include "player_manager.h"
#include "game.h"
#include "i18n.h"
#include "sdlx/font.h"
#include "sdlx/surface.h"
#include "special_owners.h"
#include "mrt/random.h"
#include "tmx/map.h"
#include "sound/mixer.h"
#include "window.h"
#include "var.h"
#include "special_zone.h"
#include "math/unary.h"
#include "player_slot.h"
#include "campaign.h"
#include "finder.h"
#include "console.h"
#include "rt_config.h"

#ifdef ENABLE_LUA
#	include "luaxx/lua_hooks.h"
#endif

IMPLEMENT_SINGLETON(GameMonitor, IGameMonitor);

IGameMonitor::IGameMonitor() : _game_over(false), _win(false), _check_items(0.5, true), _state_timer(false), _timer(0), _campaign(NULL)
#ifdef ENABLE_LUA
, lua_hooks(new LuaHooks) 
#endif
{
	on_console_slot.assign(this, &IGameMonitor::onConsole, Console->on_command);
	on_map_resize_slot.assign(this, &IGameMonitor::parseWaypoints, Map->map_resize_signal);
}

void GameItem::respawn() {
	if (spawn_limit == 0)
		return;
	
	hidden = false;
	
	LOG_DEBUG(("respawning item: %s:%s, z: %d, dir: %d", classname.c_str(), animation.c_str(), z, dir));
	Object *o = ResourceManager->createObject(classname, animation);
	if (z) 
		o->setZ(z, true);
	o->addOwner(OWNER_MAP);

	if (dir) 
		o->setDirection(dir);
	
	World->addObject(o, position.convert<float>());
	id = o->getID();
	dead_on = 0;
	if (spawn_limit > 0)
		--spawn_limit;
}

void GameItem::kill() {
	Object *o = World->getObjectByID(id);
	if (o != NULL)
		o->Object::emit("death", NULL);
}

void GameItem::setup(const std::string &name, const std::string &subname) {
	destroy_for_victory = name.substr(0, 19) == "destroy-for-victory";
	special = name.substr(0, 7) == "special";
	
	if (name == "save-for-victory")
		save_for_victory = subname;

	special |= destroy_for_victory | !save_for_victory.empty();

	size_t pos1 = name.find('(');
	if (pos1 == name.npos) 
		return;
	++pos1;

	size_t pos2 = name.find(')', pos1);
	if (pos2 == name.npos)
		return;
	--pos2;

	if (pos1 > pos2)
		return;
	
	int limit = atoi(name.substr(pos1, pos2 - pos1 + 1).c_str());
	if (limit <= 0)
		return;
	
	//LOG_DEBUG(("respawn limit = %d", limit));
	spawn_limit = limit;
}

void GameItem::renameProperty(const std::string &name) {
	Map->properties.erase(property);

	property = GameMonitor->generatePropertyName(name);
	LOG_DEBUG(("new property name %s", property.c_str()));

	updateMapProperty();
}

void GameItem::updateMapProperty() {
	std::string &prop = Map->properties[property];
	if (z) 
		prop = mrt::formatString("%d,%d,%d", position.x, position.y, z);
	else 
		prop = mrt::formatString("%d,%d", position.x, position.y);

	const Object *o = World->getObjectByID(id);
	if (o != NULL) {
		int dir = o->getDirection();
		if (dir)
			prop += mrt::formatString("/%d", dir);
	}
}

void IGameMonitor::eraseLast(const std::string &property) {
	if (_items.empty())
		throw_ex(("item list is empty!"));
	if (_items.back().property != property)
		throw_ex(("eraseLast: %s is not the latest item in list", property.c_str()));
	_items.pop_back();
}

const GameItem& IGameMonitor::find(const Object *obj) const {
	for(Items::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		const GameItem &item = *i;
		Object *o = World->getObjectByID(item.id);
		if (obj == o) 
			return item;
	}
	throw_ex(("could not find item %s:%s", obj->registered_name.c_str(), obj->animation.c_str()));
}

GameItem& IGameMonitor::find(const Object *obj) {
	for(Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		Object *o = World->getObjectByID(i->id);
		if (obj == o) 
			return *i;
	}
	throw_ex(("could not find item %s:%s", obj->registered_name.c_str(), obj->animation.c_str()));
}

GameItem& IGameMonitor::find(const std::string &property) {
	for(Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		if (i->property == property) 
			return *i;
	}
	throw_ex(("could not find item %s", property.c_str()));
}

void IGameMonitor::checkItems(const float dt) {	
	if (_game_over || !_check_items.tick(dt))
		return;
		
	int goal = 0, goal_total = 0;
	
	if (!_destroy_classes.empty()) {
		++goal_total;
		if (!World->itemExists(_destroy_classes)) 
			++goal;
	}
	
	_specials.clear();
	for(size_t i = 0; i < _external_specials.size(); ++i) {
		const int id = _external_specials[i];
		Object *o = World->getObjectByID(id);
		if (o == NULL || o->getState() == "broken")
			continue;

		v2<int> pos;
		o->getCenterPosition(pos);
		_specials.push_back(v3<int>(pos.x, pos.y, id));	
	}
	
	Uint32 ticks = SDL_GetTicks();
	
	for(Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		GameItem &item = *i;
		Object *o = World->getObjectByID(item.id);

		bool dead = true;
		if (o != NULL) {
			dead = o->getState() == "broken";
		}
		
		if (item.destroy_for_victory) {
			++goal_total;
			if (dead) {
				++goal;
			}
		} 

		if (!dead) {
			if (item.special) {
				v2<int> pos;
				o->getCenterPosition(pos);
				_specials.push_back(v3<int>(pos.x, pos.y, o->getID()));	
			}

			continue;
		}
		//object is dead.
		
		if (!item.save_for_victory.empty()) {
			gameOver("messages", item.save_for_victory, 5, false);
			continue;
		}

		if (o)
			continue;
		
		if (item.spawn_limit == 0 || item.hidden) 
			continue;
		
		if (item.dead_on == 0) {
			item.dead_on = ticks;
			LOG_DEBUG(("item %d:%s:%s is dead, log dead time.", item.id, item.classname.c_str(), item.animation.c_str()));
			continue;
		}
		
		int rt;
		Config->get("map." + item.classname + ".respawn-interval", rt, 5); 
		if (rt < 0) 
			continue;
		if (((ticks - item.dead_on) / 1000) >= (unsigned)rt) {
			//respawning item
			item.respawn();
		}
	}
	if (goal_total > 0 && goal == goal_total) {
		gameOver("messages", "mission-accomplished", 5, true);
	}
}

void IGameMonitor::add(const GameItem &item_, const bool dont_respawn) {
	GameItem item(item_);
	const bool client = PlayerManager->isClient();

#ifdef ENABLE_LUA
	if (!client && lua_hooks != NULL)
		item.hidden = !lua_hooks->on_spawn(item.classname, item.animation, item.property);
#endif

	_items.push_back(item);
	
	if (!dont_respawn && !item.hidden)
		_items.back().respawn();
}

void IGameMonitor::pushState(const std::string &state, float time) {
	if (time <= 0) 
		throw_ex(("message time <= 0 is not allowed"));
	
	_state = state;
	_state_timer.set(time);
}

const std::string IGameMonitor::popState(const float dt) {
	if (_state.empty() || !_state_timer.tick(dt))
		return std::string();
	std::string r = _state;
	_state.clear();
	return r;
}

void IGameMonitor::gameOver(const std::string &area, const std::string &message, float time, const bool win) {
	if (_game_over)
		return;
	
	_game_over = true;
	_win = win;
	displayMessage(area, message, time);
	PlayerManager->gameOver(area, message, time);
	resetTimer();
}

void IGameMonitor::displayMessage(const std::string &area, const std::string &message, float time, const bool global) {
	pushState(I18n->get(area, message), time);

	if (global && PlayerManager->isServer()) {
		if (time <= 0)
			throw_ex(("server attempts to set up %g s timer", time));
		PlayerManager->broadcastMessage(area, message, time);
	}
}
void IGameMonitor::hideMessage() {
	_state.clear();
	_timer = 0;
}

void IGameMonitor::setTimer(const std::string &area, const std::string &message, float time, const bool win_at_end) {
	_timer_message_area = area;
	_timer_message = message;
	_timer = time;
	_timer_win_at_end = win_at_end;
}

void IGameMonitor::resetTimer() {
	_timer_message.clear();
	_timer = 0;
}

void IGameMonitor::clear() {
	resetTimer();
	timers.clear();

	_game_over = false;
	_win = false;
	saveCampaign();
	_state.clear();
	
	_items.clear();
	_specials.clear();
	_external_specials.clear();
	
	_check_items.reset();
	_disabled.clear();
	_destroy_classes.clear();

	_waypoints.clear();
	_all_waypoints.clear();
	_waypoint_edges.clear();
	bonuses.clear();
}

void IGameMonitor::tick(const float dt) {	
	const bool client = PlayerManager->isClient();

#ifdef ENABLE_LUA
	if (!client && lua_hooks != NULL) {
	TRY {
		if (Map->loaded())
			lua_hooks->on_tick(dt);
	} CATCH("tick::on_tick", {
		Game->clear();
		displayMessage("errors", "script-error", 1);
		return;
	});
	processGameTimers(dt);
	}
#endif

	if (!_timer_message.empty() && _timer > 0) {
		_timer -= dt;
		if (_timer <= 0) {
			if (!client)
				gameOver(_timer_message_area, _timer_message, 5, _timer_win_at_end);
			_timer = 0;
		}
	}

	std::string game_state = popState(dt);
	if (_game_over && !game_state.empty()) {
#ifdef ENABLE_LUA
	if (!client && lua_hooks != NULL) {
	TRY {
		std::string next_map = lua_hooks->getNextMap();
		if (!next_map.empty()) {
			lua_hooks->resetNextMap();
			startGame(_campaign, next_map);
			return;
		}
	} CATCH("tick::game_over", {
		Game->clear();
		displayMessage("errors", "script-error", 1);
		return;
	});
	}
#endif
		saveCampaign();
		Game->clear();
	}
}

void IGameMonitor::render(sdlx::Surface &window) {
	static const sdlx::Font * _big_font;
	if (_big_font == NULL)
		_big_font = ResourceManager->loadFont("big", true);

	if (!_state.empty()) {
		int w = _big_font->render(NULL, 0, 0, _state), h = _big_font->getHeight();
		_state_bg.init("menu/background_box.png", window.getWidth() + 32, h); //fixme
		
		int x = (window.getWidth() - w) / 2;
		//int y = (window.getHeight() - _big_font->getHeight()) / 2;
		int y = window.getHeight() - _big_font->getHeight() - 32;
		_state_bg.render(window, (window.getWidth() - _state_bg.w) / 2, y + (h - _state_bg.h) / 2);
		_big_font->render(window, x, y, _state);
	}

	if (_timer > 0) {
		int m = (int)_timer / 60;
		int ms = (int)(10 * (_timer - (int)_timer));
		std::string timer_str; 
		if (m) {
			timer_str = mrt::formatString("%2d%c%02d", m, (ms / 2 == 0 || ms /2 == 1 || ms / 2 == 4)?':':'.', ((int)_timer) % 60);
		} else 
			timer_str = mrt::formatString("   %2d.%d", (int)_timer, ms);
		
		int tw = timer_str.size() + 1;
		_big_font->render(window, window.getWidth() - _big_font->getWidth() * tw, 
			 window.getHeight() - _big_font->getHeight() * 3 / 2, 
			 timer_str);
	}

}


const bool IGameMonitor::disabled(const Object *o) const {
	return _disabled.find(o->classname) != _disabled.end() || _disabled.find(o->registered_name) != _disabled.end();
}

void IGameMonitor::disable(const std::string &classname, const bool value) {
	LOG_DEBUG(("%s ai for classname %s", value?"disabling":"enabling", classname.c_str()));
	if (value) {
		_disabled.insert(classname);
	} else {
		_disabled.erase(classname);
	}
}


#include "mrt/serializator.h"

void IGameMonitor::serialize(mrt::Serializator &s) const {
TRY {
	s.add(_game_over);
	s.add(_specials);

	if (_game_over) {
		s.add(_state);
		s.add(_state_timer);
	}
	
	s.add(_timer_message);
	s.add(_timer_message_area);
	s.add(_timer);

	s.add(_disabled);
	s.add(_destroy_classes);

} CATCH("serialize", throw);
}

void IGameMonitor::deserialize(const mrt::Serializator &s) {
TRY {
	s.get(_game_over);
	s.get(_specials);

	if (_game_over) {
		s.get(_state);
		s.get(_state_timer);
	}
	
	s.get(_timer_message);
	s.get(_timer_message_area);
	s.get(_timer);

	s.get(_disabled);
	s.get(_destroy_classes);
	
} CATCH("deserialize", throw);
}

void IGameMonitor::killAllClasses(const std::set<std::string> &classes) {
	_destroy_classes = classes;
}

const bool IGameMonitor::hasWaypoints(const std::string &classname) const {
	WaypointClassMap::const_iterator wp_class = _waypoints.find(classname);
	if (wp_class == _waypoints.end() && classname.compare(0, 7, "static-") == 0)  //no matter static or not
		wp_class = _waypoints.find(classname.substr(7));

	return (wp_class != _waypoints.end());	
}

const std::string IGameMonitor::getRandomWaypoint(const std::string &classname, const std::string &last_wp) const {
	if (last_wp.empty()) 
		throw_ex(("getRandomWaypoint('%s', '%s') called with empty name", classname.c_str(), last_wp.c_str()));
	
	WaypointClassMap::const_iterator wp_class = _waypoints.find(classname);
	if (wp_class == _waypoints.end() && classname.compare(0, 7, "static-") == 0)  //no matter static or not
		wp_class = _waypoints.find(classname.substr(7));

	if (wp_class == _waypoints.end()) 
		throw_ex(("no waypoints for '%s' defined", classname.c_str()));
		
	WaypointEdgeMap::const_iterator b = _waypoint_edges.lower_bound(last_wp);
	WaypointEdgeMap::const_iterator e = _waypoint_edges.upper_bound(last_wp);
	if (b == e) 
		throw_ex(("no edges defined for waypoint '%s'", last_wp.c_str()));

	int wp = mrt::random(_waypoint_edges.size() * 2);
	while(true) {
		for(WaypointEdgeMap::const_iterator i = b; i != e; ++i) {
			if (wp-- <= 0) {
				return i->second;
			}
		}
	}
	throw_ex(("getRandomWaypoint(unexpected termination)"));
	return "*bug*";
}

const std::string IGameMonitor::getNearestWaypoint(const BaseObject *obj, const std::string &classname) const {
	v2<int> pos;
	obj->getPosition(pos);
	int distance = -1;
	std::string wp;
	
	WaypointClassMap::const_iterator i = _waypoints.find(classname);
	if (i == _waypoints.end() && classname.compare(0, 7, "static-") == 0)  //no matter static or not
		i = _waypoints.find(classname.substr(7));
	
	if (i == _waypoints.end())
		throw_ex(("no waypoints for '%s' found", classname.c_str()));

	for(WaypointMap::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
		int d = j->second.quick_distance(pos);
		if (distance == -1 || d < distance) {
			distance = d;
			wp = j->first;
		}
	}
	return wp;
}


void IGameMonitor::getWaypoint(v2<float> &wp, const std::string &classname, const std::string &name) {
	if (name.empty() || classname.empty()) 
		throw_ex(("getWaypoint('%s', '%s') called with empty classname and/or name", classname.c_str(), name.c_str()));
	
	WaypointClassMap::const_iterator wp_class = _waypoints.find(classname);
	if (wp_class == _waypoints.end() && classname.compare(0, 7, "static-") == 0)  //no matter static or not
		wp_class = _waypoints.find(classname.substr(7));
	if (wp_class == _waypoints.end()) 
		throw_ex(("no waypoints for '%s' defined", classname.c_str()));
	
	WaypointMap::const_iterator i = wp_class->second.find(name);
	if (i == wp_class->second.end())
		throw_ex(("no waypoints '%s' defined", name.c_str()));
	wp = i->second.convert<float>();
}

void IGameMonitor::renderWaypoints(sdlx::Surface &surface, const sdlx::Rect &src, const sdlx::Rect &dst) {
	const sdlx::Surface *s = ResourceManager->loadSurface("car-waypoint.png");
	
	for(WaypointClassMap::const_iterator i = _waypoints.begin(); i != _waypoints.end(); ++i) {
		//const std::string &classname = i->first;
		for(WaypointMap::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			const v2<int> &wp = j->second;
			surface.copyFrom(*s, 
			wp.x - src.x + dst.x, 
			wp.y - src.y + dst.y - s->getHeight());	
		}
	}
	
	s = ResourceManager->loadSurface("edge.png");
	int w = s->getWidth() / 3, h = s->getHeight();
	sdlx::Rect normal(0, 0, w, h), out(w, 0, w, h), in(2 * w, 0, w, h);
	
	for(WaypointEdgeMap::const_iterator i = _waypoint_edges.begin(); i != _waypoint_edges.end(); ++i) {
		WaypointMap::const_iterator a = _all_waypoints.find(i->first);
		if (a == _all_waypoints.end()) 
			throw_ex(("no waypoint '%s' defined", i->first.c_str()));
		WaypointMap::const_iterator b = _all_waypoints.find(i->second);
		if (b == _all_waypoints.end()) 
			throw_ex(("no waypoint '%s' defined", i->second.c_str()));
		
		const v2<float> ap = a->second.convert<float>();
		const v2<float> bp = b->second.convert<float>();
		//LOG_DEBUG(("%d:%d -> %d:%d", ap.x, ap.y, bp.x, bp.y));
		v2<float> p = ap, d = bp - ap;
		d.normalize();
		p += d * w;
		int len0 = (int)ap.distance(bp); 
		for(int len = len0; len > w; len -= w, p += d * w) {
			const sdlx::Rect &r = (len == len0)? out: (len <= 2 * w ? in:normal );
			surface.copyFrom(*s, r, 
			(int)(p.x - src.x + dst.x + d.x), 
			(int)(p.y - src.y + dst.y + d.y));
		}
	}
}

template<typename T>
static void coord2v(T &pos, const std::string &str) {
	std::string pos_str = str;

	const bool tiled_pos = pos_str[0] == '@';
	if (tiled_pos) { 
		pos_str = pos_str.substr(1);
	}

	TRY {
		pos.fromString(pos_str);
	} CATCH(mrt::formatString("parsing '%s'", str.c_str()).c_str() , throw;)

	if (tiled_pos) {
		v2<int> tile_size = Map->getTileSize();
		pos.x *= tile_size.x;
		pos.y *= tile_size.y;
		//keep z untouched.
	}
}

void IGameMonitor::loadMap(Campaign *campaign, const std::string &name, const bool spawn_objects, const bool skip_loadmap) {
	_campaign = campaign;
	const bool client = PlayerManager->isClient();

	IMap &map = *IMap::get_instance();

	if (!skip_loadmap) {
		map.load(name);
	} else {
		if (!map.loaded())
			throw_ex(("loadMap() called with skip Map::load() flag. Map must be initialized at this point."));
	}

	ResourceManager->preload();

	_waypoints.clear();
	_waypoint_edges.clear();
	
	Config->clearOverrides();

#	ifdef ENABLE_LUA
		if (lua_hooks)
			lua_hooks->clear();
#	endif

	std::string script = Finder->find("maps/" + name + ".lua", false);

	if (!skip_loadmap && !script.empty() && !Map->soloAwareMode()) {
#	ifdef ENABLE_LUA	
		TRY {
			if (lua_hooks) {
				lua_hooks->load(script);
			}
		} CATCH("loadMap::load", {
			Game->clear();
			displayMessage("errors", "script-error", 1);
			return;
		});
#	else
		throw_ex(("this map requires lua scripting support."));
#	endif
	}

	//difficulty settings
	int difficulty = 2; //map as is == hard, default: normal

	if (campaign) {
		Config->get("campaign." + campaign->name + ".difficulty", difficulty, 1);

		Var v_true("bool");
		v_true.b = (difficulty >= 3);
		Config->setOverride("engine.fog-of-war.enabled", v_true);
	}


	//const v2<int> size = map.getSize();
	for (IMap::PropertyMap::iterator i = map.properties.begin(); i != map.properties.end(); ++i) {
		if (i->first.empty())
			throw_ex(("property name could not be empty"));
		
		std::vector<std::string> res;
		mrt::split(res, i->first, ":");
		const std::string &type = res[0];
		
		if (type == "waypoint" || type == "edge") //save some time
			continue;

		if (type != "spawn" && type != "object" && type != "config" && 
			type != "zone" && type != "ambient-sound")
				throw_ex(("unsupported line: '%s'", i->first.c_str()));
		
		if (!spawn_objects && type != "config")
			continue;
	
		if (type == "ambient-sound") {
			Mixer->startAmbient(i->second);
			continue;
		}
	
		v3<int> pos;
		int dir = 0;
		if (type != "edge" && type != "config") {
			std::string::size_type dp = i->second.rfind('/');
			if (dp != std::string::npos && dp + 1 < i->second.size()) {
				dir = atoi(i->second.substr(dp + 1).c_str());
				LOG_DEBUG(("found argument %d", dir));
			}
			coord2v< v3<int> >(pos, i->second.substr(0, dp));
		}
	
		/*
		if (pos.x < 0) 
			pos.x += size.x;
		if (pos.y < 0) 
			pos.y += size.y;
		*/
		
		if (type == "spawn") {
			LOG_DEBUG(("spawnpoint: %d,%d,%d", pos.x, pos.y, pos.z));
			v2<int> tile_size = Map->getTileSize();
			pos.x += tile_size.x / 2;
			pos.y += tile_size.y / 2;
			PlayerManager->addSlot(pos);
		} else {
			if (type == "object") {
				LOG_DEBUG(("spawning: object %s, animation %s, pos: %s", res[1].c_str(), res[2].c_str(), i->second.c_str()));
				if (res.size() < 4)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				//LOG_DEBUG(("name: %s", res[3].c_str()));
				res.resize(5);
				GameItem item(res[1], res[2], i->first, v2<int>(pos.x, pos.y), pos.z);
				item.setup(res[3], res[4]);
				item.dir = dir;
				
				add(item, true);
			} else if (type == "config") {
				if (res.size() < 2)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				
				std::vector<std::string> value;
				mrt::split(value, i->second, ":");
				value.resize(2);
				if (value[0] != "int" && value[0] != "float" && value[0] != "string" && value[0] != "bool")
					throw_ex(("cannot set config variable '%s' of type '%s'", res[1].c_str(), value[0].c_str()));

				const std::string &name = res[1];
				if (difficulty == 0 && name == "map.spawn-limit") {
					LOG_DEBUG(("skipping spawn limit [difficulty]"));
					continue;
				}
				
				Var var(value[0]);
				var.fromString(value[1]);

				if (difficulty <= 1 && name.compare(0, 4, "map.") == 0) { //easy + normal
					//-item.respawn-interval
					std::vector<std::string> key_names;
					mrt::split(key_names, name, ".");
					if (key_names.size() > 2 && key_names[2] == "respawn-interval") {
						const std::string &item_name = key_names[1];
						if ((var.i < 0 || var.i >= 10000) &&
							(
								(item_name.size() > 5 && item_name.compare(item_name.size() - 5, 5, "-item") == 0) ||
								item_name == "megaheal" || item_name == "heal"
							)
						) { //stupid vz! :)
							LOG_DEBUG(("skipping: '%s' = %d override [difficulty]", name.c_str(), var.i));
							continue;
						}
					}
				}

				Config->setOverride(name, var);
			} else if (type == "zone") {
				LOG_DEBUG(("%s %s %s", type.c_str(), i->first.c_str(), i->second.c_str()));
				std::vector<std::string> value;
				mrt::split(value, i->second, ":");
				if (value.size() < 2)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				v3<int> pos;
				v2<int> size;
				coord2v(pos, value[0]);
				coord2v(size, value[1]);
				res.resize(4);
				
				SpecialZone zone(ZBox(pos, size), res[1], res[2], res[3]);
				zone.area = "hints/" + name;
				PlayerManager->addSpecialZone(zone);
			} 
		}
	}
	
	if (Config->has("map.kill-em-all")) {
		std::string cstr;
		Config->get("map.kill-em-all", cstr, std::string());
		std::vector<std::string> res;
		mrt::split(res, cstr, ",");
		
		std::set<std::string> classes;
		for(size_t i = 0; i < res.size(); ++i) {
			std::string &str = res[i];
			mrt::trim(str);
			if (!str.empty())
				classes.insert(str);
		}
		killAllClasses(classes);
		LOG_DEBUG(("kill'em all classes: %u", (unsigned)classes.size()));
	}


	if (RTConfig->game_type == GameTypeDeathMatch && Config->has("multiplayer.time-limit")) {
		int tl; 
		Config->get("multiplayer.time-limit", tl, 0);
		if (tl != 0) 
			setTimer("messages", "time-limit-reached", (float)tl, false);
	}
	
	LOG_DEBUG(("generating matrixes"));
	Map->generateMatrixes();
	
	parseWaypoints(0,0,0,0);
	
	Config->invalidateCachedValues();
	
	GET_CONFIG_VALUE("engine.max-time-slice", float, mts, 0.025);
	World->setTimeSlice(mts);
	
	for(Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		if (!i->hidden)
			i->respawn();
	}

#	ifdef ENABLE_LUA
	TRY {
		if (!client && lua_hooks)
			lua_hooks->on_load();
	} CATCH("loadMap::on_load", {
		Game->clear();
		displayMessage("errors", "script-error", 1);
		return;
	});

#	endif

	Window->resetTimer();
}

void IGameMonitor::parseWaypoints(int, int, int, int) {
	LOG_DEBUG(("parsing waypoints..."));
	IMap &map = *IMap::get_instance();
	v3<int> pos;
	
	_waypoints.clear();
	_all_waypoints.clear();
	_waypoint_edges.clear();
	
	for (IMap::PropertyMap::iterator i = map.properties.begin(); i != map.properties.end(); ++i) {
		if (i->first.empty())
			throw_ex(("property name could not be empty"));
		
		std::vector<std::string> res;
		mrt::split(res, i->first, ":");
		const std::string &type = res[0];

		if (type == "waypoint") {
			if (res.size() < 3)
				throw_ex(("'%s' misses an argument", i->first.c_str()));
			v2<int> tile_size = Map->getTileSize(); //tiled correction
			coord2v< v3<int> >(pos, i->second);
			pos.x += tile_size.x / 2;
			pos.y += tile_size.y / 2;
			LOG_DEBUG(("waypoint class %s, name %s : %d,%d", res[1].c_str(), res[2].c_str(), pos.x, pos.y));
			_waypoints[res[1]][res[2]] = v2<int>(pos.x, pos.y);
			_all_waypoints[res[2]] = v2<int>(pos.x, pos.y);
		} else if (type == "edge") {
			if (res.size() < 3)
				throw_ex(("'%s' misses an argument", i->first.c_str()));
			if (res[1] == res[2])
				throw_ex(("map contains edge from/to the same vertex"));
			_waypoint_edges.insert(WaypointEdgeMap::value_type(res[1], res[2]));
		} 
	}

	LOG_DEBUG(("checking waypoint graph..."));
	for(WaypointEdgeMap::const_iterator i = _waypoint_edges.begin(); i != _waypoint_edges.end(); ++i) {
		const std::string &dst = i->second;
		WaypointEdgeMap::const_iterator b = _waypoint_edges.lower_bound(dst);
		if (b == _waypoint_edges.end() || b->first != dst)
			throw_ex(("no edges out of waypoint '%s'", dst.c_str()));
	}
	LOG_DEBUG(("%u items on map, %u waypoint classes, %u edges", (unsigned)getItemsCount(), (unsigned)_waypoints.size(), (unsigned)_waypoint_edges.size()));
}

const std::string IGameMonitor::generatePropertyName(const std::string &prefix) {
	//LOG_DEBUG(("prefix: %s", prefix.c_str()));
	IMap::PropertyMap::const_iterator b = Map->properties.lower_bound(prefix);
	int n = 0;
	
	for(IMap::PropertyMap::const_iterator i = b; i != Map->properties.end(); ++i) {
		if (i->first.compare(0, prefix.size(), prefix) != 0) 
			continue;
		std::string suffix = i->first.substr(prefix.size());
		if (!suffix.empty() && suffix[0] == ':') {
			int i = atoi(suffix.c_str() + 1);
			if (i > n) 
				n = i;
		}
	}
	
	++n;

	std::string name =  mrt::formatString("%s:%d", prefix.c_str(), n);
	if (Map->properties.find(name) != Map->properties.end()) 
		throw_ex(("failed to generate unique name. prefix: %s, n: %d", prefix.c_str(), n));
	return name;
}

void IGameMonitor::addBonuses(const PlayerSlot &slot) {
	if (_campaign == NULL)
		return;
	Object *o = slot.getObject();
	if (o == NULL)
		return;
	const std::vector<Campaign::ShopItem> & wares = _campaign->wares;
	
	bool first_time = bonuses.empty();
	size_t idx = 0;
	
	for(std::vector<Campaign::ShopItem>::const_iterator i = wares.begin(); i != wares.end(); ++i) {
		int n = i->amount;
		if (n <= 0 || i->object.empty() || i->animation.empty())
			continue;
		LOG_DEBUG(("adding bonus: %s", i->name.c_str()));
		int dirs = (n > 8)?16:(n > 4 ? 8: 4);
		for(int d = 0; d < n; ++d) {
			v2<float> dir; 
			dir.fromDirection(d % dirs, dirs);
			dir *= o->size.length();
			//LOG_DEBUG(("%g %g", d.x, d.y));
			if (first_time) 
				bonuses.push_back(GameBonus(i->object + "(ally)", i->animation, 0));
			if (World->getObjectByID(bonuses[idx].id) == NULL) {
				Object *bonus = o->spawn(bonuses[idx].classname, bonuses[idx].animation, dir, v2<float>());
				bonuses[idx].id = bonus->getID();
			}
			++idx;
		}
	}
}

#include "nickname.h"
#include "rt_config.h"

void IGameMonitor::startGame(Campaign *campaign, const std::string &name) {
	Game->clear();
	PlayerManager->startServer();
	GameMonitor->loadMap(campaign, name);
	if (!Map->loaded())
		return; //error 

	if (PlayerManager->getSlotsCount() <= 0)
		throw_ex(("no slots available on map"));
	
	if (RTConfig->server_mode)
		return;
	
	PlayerSlot &slot = PlayerManager->getSlot(0);
	std::string cm;
	Config->get("player.control-method", cm, "keys");
	Config->get("player.name-1", slot.name, Nickname::generate());
	slot.createControlMethod(cm);

	std::string object, vehicle;
	PlayerManager->getDefaultVehicle(object, vehicle);

	slot.spawnPlayer(object, vehicle);
	PlayerManager->getSlot(0).setViewport(Window->getSize());
}

IGameMonitor::~IGameMonitor() {
#ifdef ENABLE_LUA
	delete lua_hooks;
#endif
}

void IGameMonitor::onTooltip(const std::string &event, const int slot_id, const std::string &area, const std::string &message) {
#ifdef ENABLE_LUA
	if (lua_hooks)
		lua_hooks->on_tooltip(event, slot_id, area, message);
#endif
}

void IGameMonitor::onScriptZone(const int slot_id, const SpecialZone &zone, const bool global) {
	const bool client = PlayerManager->isClient();
	if (client)
		return;
	
#ifndef ENABLE_LUA
	throw_ex(("no script support compiled in."));
#else 
	TRY {
		if (lua_hooks == NULL)
			throw_ex(("lua hooks was not initialized"));
		if (global)
			lua_hooks->call(zone.name);
		else 
			lua_hooks->call1(zone.name, slot_id + 1);
	} CATCH("onScriptZone", {
		Game->clear();
		displayMessage("errors", "script-error", 1);
		return;
	});

#endif
}

const std::string IGameMonitor::onConsole(const std::string &cmd, const std::string &param) {
#ifdef ENABLE_LUA
	if (cmd == "call") {
		try {
			if (lua_hooks == NULL)
				throw_ex(("lua hooks was not initialized"));
			lua_hooks->call(param);
		} catch(const std::exception &e) {
			return std::string("error") + e.what();
		}
		return "ok";
	}
#endif
	return std::string();
}

const bool IGameMonitor::usedInCampaign(const std::string &base, const std::string &id) const {
	return used_maps.find(std::pair<std::string, std::string>(base, id)) != used_maps.end();
}

const void IGameMonitor::useInCampaign(const std::string &base, const std::string &id) {
	used_maps.insert(std::pair<std::string, std::string>(base, id));
}

void IGameMonitor::saveCampaign() {
	if (_campaign == NULL) 
		return;

	LOG_DEBUG(("saving compaign state..."));
	const std::string mname = "campaign." + _campaign->name + ".maps." + Map->getName();
	
	if (PlayerManager->getSlotsCount()) {
		PlayerSlot &slot = PlayerManager->getSlot(0); 
		int score; 
		Config->get("campaign." + _campaign->name + ".score", score, 0);
		score += slot.score;
		Config->set("campaign." + _campaign->name + ".score", score);
		LOG_DEBUG(("total score: %d", score));

		int mscore;
		Config->get(mname + ".maximum-score", mscore, 0);
		if (slot.score > mscore) 
			Config->set(mname + ".maximum-score", slot.score);
	}
			
	bool win;
	Config->get(mname + ".win", win, false);
	if (_win) {
		Config->set(mname + ".win", _win);
		_campaign->clearBonuses();
	} 
			
	_campaign = NULL;	
}

void IGameMonitor::startGameTimer(const std::string &name, const float period, const bool repeat) {
#ifdef ENABLE_LUA
	LOG_DEBUG(("starting timer '%s', %g sec., repeat: %s", name.c_str(), period, repeat?"yes":"no"));
	timers.insert(Timers::value_type(name, Timer(period, repeat)));
#endif
}

void IGameMonitor::stopGameTimer(const std::string &name) {
#ifdef ENABLE_LUA
	timers.erase(name);
#endif
}

void IGameMonitor::processGameTimers(const float dt) {
#ifdef ENABLE_LUA
	if (lua_hooks == NULL)
		return;
	for(Timers::iterator i = timers.begin(); i != timers.end(); ) {
		Timer & timer = i->second;
		timer.t += dt;
		if (timer.t >= timer.period) {
			//triggering event
			std::string name = i->first;
			
			if (timer.repeat) {
				while(timer.t >= timer.period) timer.t -= timer.period;
				++i;
			} else {
				//one shot
				timers.erase(i++);
			}

			TRY {
				LOG_DEBUG(("calling on_timer(%s)", name.c_str()));
				lua_hooks->on_timer(name); //callback could add/delete timers!!
			} CATCH("processGameTimers", );
			
		} else {
			++i; //continue;
		}
	}
#endif
}

