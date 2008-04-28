
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

#include "player_manager.h"
#include "player_slot.h"
#include "object.h"
#include "world.h"
#include "config.h"
#include "resource_manager.h"
#include "version.h"
#include "game.h"
#include "window.h"
#include "game_monitor.h"
#include "special_zone.h"
#include "nickname.h"
#include "menu/tooltip.h"
#include "menu/chat.h"

#include "controls/control_method.h"

#include "net/server.h"
#include "net/client.h"
#include "net/message.h"

#include "sound/mixer.h"
#include "sdlx/surface.h"
#include "tmx/map.h"

#include "mrt/random.h"

#include "math/unary.h"
#include "math/binary.h"
#include "mrt/utf8_utils.h"

#include "sl08/sl08.h"
#include "rt_config.h"


IMPLEMENT_SINGLETON(PlayerManager, IPlayerManager);


const int IPlayerManager::onConnect(Message &message) {
	/*
	const std::string an = "red-tank";
	LOG_DEBUG(("new client! spawning player:%s", an.c_str()));
	const int client_id = spawnPlayer("tank", an, "network");
	LOG_DEBUG(("client #%d", client_id));
	*/
	const int client_id = findEmptySlot();
	
	LOG_DEBUG(("sending server status message..."));
	message.type = Message::ServerStatus;
	message.set("version", getVersion());
	bool fog;
	Config->get("engine.fog-of-war.enabled", fog, false);
	if (fog)
		message.set("fog", "o'war");

	mrt::Serializator s;
	Map->serialize(s);
	
	//s.add(client_id);
	//_players[client_id].position.serialize(s);
	
	serializeSlots(s);	

	s.finalize(message.data);
	LOG_DEBUG(("server status message size = %u", (unsigned) message.data.getSize()));

	//LOG_DEBUG(("world: %s", message.data.dump().c_str()));
	return client_id;
}

void IPlayerManager::onDisconnect(const int cid) {
	for(size_t i = 0; i < _players.size(); ++i) {
		PlayerSlot &slot = _players[i];
		if (slot.remote != cid)
			continue;
		
		Object *obj = slot.getObject();
		if (obj)
			obj->Object::emit("death", NULL);
	
		slot.clear();
	}
}

#include "var.h"

void IPlayerManager::onMessage(const int cid, const Message &message, const int timestamp) {
TRY {
	int now = SDL_GetTicks();
	LOG_DEBUG(("incoming message %s, incoming timestamp: %d, my timestamp: %d, delta + ping: %+d", message.getType(), timestamp, now, timestamp - now));
	switch(message.type) {
	case Message::ServerStatus: {
		LOG_DEBUG(("server version: %s", message.get("version").c_str()));
		LOG_DEBUG(("loading map..."));
		
		mrt::Serializator s(&message.data);
		Map->deserialize(s);
		GameMonitor->loadMap(NULL, Map->getName(), false, true);
		
		if (message.has("fog")) {
			Var v_true("bool");
			v_true.b = true;
			Config->setOverride("engine.fog-of-war.enabled", v_true);
		}
		
		int n;
		s.get(n);
		
		_players.clear();
		for(int i = 0; i < n; ++i) {
			PlayerSlot slot;
			slot.deserialize(s);
			_players.push_back(slot);
		}
		
		
		for(size_t i = 0; i < _local_clients; ++i) {
			Message m(Message::RequestPlayer);

			std::string vehicle;
			Config->get(mrt::formatString("menu.default-vehicle-%u", (unsigned)(i + 1)), vehicle, "tank");
			m.set("vehicle", vehicle);

			std::string name;
			Config->get(mrt::formatString("player.name-%u", (unsigned)(i + 1)), name, Nickname::generate());
			m.set("name", name);
			_client->send(m);
		}	
		
		_next_ping = 0;
		_ping = true;
		break;	
	}
	
	case Message::RequestPlayer: {
		int id = findEmptySlot();
		PlayerSlot &slot = _players[id];
		
		std::string vehicle(message.get("vehicle")), animation;

		slot.getDefaultVehicle(vehicle, animation);
		slot.name = message.get("name");
		mrt::utf8_resize(slot.name, 32);
		LOG_DEBUG(("player%d: %s:%s, name: %s", id, vehicle.c_str(), animation.c_str(), slot.name.c_str()));

		slot.remote = cid;
		
		if (RTConfig->game_type == GameTypeCTF || RTConfig->game_type == GameTypeTeamDeathMatch) {
			std::string team = message.get("team"); //just a little reminder 
		}
		
		slot.spawnPlayer(id, vehicle, animation);

		mrt::Serializator s;
		World->serialize(s);
		serializeSlots(s);
		
		Message m(Message::GameJoined);
		m.channel = id;
		s.finalize(m.data);
		_server->send(cid, m);

		Window->resetTimer();
		break;
	}
	
	case Message::GameJoined: {
		int id = message.channel;
		if (id < 0 || (unsigned)id >= _players.size())
			throw_ex(("player id exceeds players count (%d/%d)", id, (int)_players.size()));
		LOG_DEBUG(("players = %u, slot: %d", (unsigned)_players.size(), id));
		PlayerSlot &slot = _players[id];
		
		slot.viewport.reset();
		slot.visible = true;
		slot.remote = id;

		assert(slot.control_method == NULL);
		if (_local_clients == 1) {
			GET_CONFIG_VALUE("player.control-method", std::string, control_method, "keys");	
			slot.createControlMethod(control_method);
		} else if (_local_clients == 2) {
			size_t idx = 0;
			for(size_t i = 0; i < _players.size(); ++i) {
				if (_players[i].remote != -1)
					++idx;
			}
			assert(idx == 1 || idx == 2);
			std::string control_method;
			Config->get(mrt::formatString("player.control-method-%u", (unsigned)idx), control_method, mrt::formatString("keys-%u", (unsigned)idx));	
			slot.createControlMethod(control_method);
		} else throw_ex(("cannot handle %u clients", (unsigned)_local_clients));
		
		mrt::Serializator s(&message.data);
		World->deserialize(s);
		deserializeSlots(s);

		Window->resetTimer();
		_game_joined = true;
		break;
	}
	
	case Message::UpdateWorld: {
		mrt::Serializator s(&message.data);
		deserializeSlots(s);
		float dt = (now + _net_stats.getDelta() - timestamp) / 1000.0f;
		LOG_DEBUG(("update world, delta: %+d, dt: %g", _net_stats.getDelta(), dt));
		World->applyUpdate(s, dt, message.has("sync"));
		GameMonitor->deserialize(s);
		break;
	} 
	case Message::RequestObjects: {
		assert(_server != NULL);
		
		int first_id;
		{
			mrt::Serializator md(&message.data);
			md.get(first_id);
		}

		mrt::Serializator s;
		serializeSlots(s);
		World->generateUpdate(s, false, first_id);
		GameMonitor->serialize(s);
			
		Message m(Message::UpdateWorld);
		m.set("sync", "1");
		s.finalize(m.data);
		_server->send(cid, m);
		break;
	}
	case Message::PlayerState: {
		int id = message.channel;
		if (id < 0 || (unsigned)id >= _players.size())
			throw_ex(("player id exceeds players count (%d/%d)", id, (int)_players.size()));
		PlayerSlot &slot = _players[id];
		if (slot.remote != cid)
			throw_ex(("client in connection %d sent wrong channel id %d", cid, id));

/*		ExternalControl * ex = dynamic_cast<ExternalControl *>(slot.control_method);
		if (ex == NULL)
			throw_ex(("player with id %d uses non-external control method", id));
		
		ex->state.deserialize(s);
		*/
		mrt::Serializator s(&message.data);

		PlayerState state;
		state.deserialize(s);
		if (slot.spectator) {
			LOG_DEBUG(("activity in spectator slot: %s", state.dump().c_str()));
			slot.old_state = state;
			break;
		}

		Object * obj = slot.getObject();
		if (obj == NULL) {
			LOG_WARN(("player state for non-existent object %d recv'ed", slot.id));
			break;
		}

		assert(slot.id == obj->getID());
		//obj->uninterpolate();
		//obj->interpolate();
		
		float dt = (now + slot.net_stats.getDelta() - timestamp ) / 1000.0f; 
		LOG_DEBUG(("player state, delta: %+d, dt: %g", slot.net_stats.getDelta(), dt));
		if (dt < 0) 
			dt = 0;
		World->tick(*obj, -dt, false);
		
		obj->updatePlayerState(state);
		slot.need_sync = true;
		
		World->tick(*obj, dt);
		//World->interpolateObject(obj);
		break;
	} 
	case Message::UpdatePlayers: { 
		mrt::Serializator s(&message.data);
		ObjectMap updated_objects, interpolated_objects;

		float dt = (now + _net_stats.getDelta() - timestamp) / 1000.0f;
		LOG_DEBUG(("update players, delta: %+d, dt: %g", _net_stats.getDelta(), dt));
		if (dt < 0) 
			dt = 0;

		while(!s.end()) {
			int id;
			s.get(id);
			PlayerSlot *slot = getSlotByID(id);
			bool my_state = false;
			
			if (slot != NULL) {
				my_state = slot->visible;
			} 
			
			Object *o = World->getObjectByID(id);

			PlayerState state; 
			state.deserialize(s);
			
			World->deserializeObjectPV(s, o);
			bool dont_interpolate;
			s.get(dont_interpolate);
			
			if (o == NULL) {
				LOG_WARN(("nothing known about object id %d now, skip update", id));
				continue;
			}

			LOG_DEBUG(("id: %d, state: %s %s (my state: %s) %s", 
				id, state.dump().c_str(), my_state?"[skipped]":"", o->getPlayerState().dump().c_str(), 
				(my_state && state != o->getPlayerState())?"**DIFFERS**":""));

			if (!my_state)
				o->updatePlayerState(state); //update states for all players but me.

			updated_objects.insert(ObjectMap::value_type(o->getID(), o));
			if (!dont_interpolate)
				interpolated_objects.insert(ObjectMap::value_type(o->getID(), o));
			else o->uninterpolate();
		}
		World->tick(updated_objects, dt);
		World->interpolateObjects(interpolated_objects);
		break;
	} 

	case Message::Ping: {
		mrt::Serializator out, in(&message.data);
		
		unsigned client_ts;
		int client_delta;
		in.get(client_ts);
		in.get(client_delta);
		unsigned server_ts = now;

		LOG_DEBUG(("ping: timestamps delta: %+d, client delta: %+d", client_ts - server_ts, client_delta));

		out.add(client_ts);
		out.add(server_ts);
		
		int delta = 0;

		for(size_t id = 0; id < _players.size(); ++id) {
			PlayerSlot &slot = _players[id];
			if (slot.remote != cid)
				continue;
			
			if (client_delta) 
				slot.net_stats.updateDelta(-client_delta);
			delta = slot.net_stats.getDelta();
		}
		
		out.add(delta);

		Message m(Message::Pang);
		out.finalize(m.data);
				
		_server->send(cid, m);
		break;
	}
	
	case Message::Pang: {
		mrt::Serializator out, in(&message.data);
		unsigned old_client_ts, server_ts, client_ts = now;
		int server_delta;
		in.get(old_client_ts);
		in.get(server_ts);
		in.get(server_delta);

		float ping = (client_ts - old_client_ts) / 2.0f;
		if (ping < 0) 
			throw_ex(("bogus timestamp sent: %u", server_ts));

		ping = _net_stats.updatePing(ping);
		int delta = server_ts - (client_ts + old_client_ts) / 2;
		
		LOG_DEBUG(("pang: delta: %+d, server delta: %+d", delta, server_delta));
		
		_net_stats.updateDelta(delta);
		if (server_delta)
			_net_stats.updateDelta(-server_delta);

		LOG_DEBUG(("ping: %g, delta: %d", _net_stats.getPing(), _net_stats.getDelta()));
		
		GET_CONFIG_VALUE("multiplayer.ping-interval", int, ping_interval, 1500);

		_next_ping = now + ping_interval; 
		

		out.add(client_ts);
		out.add(server_ts);
		out.add(_net_stats.getDelta());

		Message m(Message::Pong);
		out.finalize(m.data);
		_client->send(m);
	} break;
	
	case Message::Pong: {
		mrt::Serializator in(&message.data);
		unsigned client_ts, old_server_ts, server_ts = now;
		int client_delta;
		in.get(client_ts);
		in.get(old_server_ts);
		in.get(client_delta);

		float ping = (server_ts - old_server_ts) / 2.0f;
		if (ping < 0) 
			throw_ex(("bogus timestamp sent: %u", server_ts));

		int delta = client_ts - (server_ts + old_server_ts) / 2;
		LOG_DEBUG(("pong: delta: %+d, client delta: %+d", delta, client_delta));
	
		for(size_t id = 0; id < _players.size(); ++id) {
			PlayerSlot &slot = _players[id];
			if (slot.remote != cid)
				continue;
		
			slot.net_stats.updatePing(ping);
			slot.net_stats.updateDelta(delta);
			if (client_delta)
				slot.net_stats.updateDelta(-client_delta);
			LOG_DEBUG(("player %u: ping: %g ms, delta: %+d", (unsigned)id, slot.net_stats.getPing(), slot.net_stats.getDelta()));		
		}
	} break;

	
	case Message::Respawn: {
		TRY {
		//int id = message.channel;
		//if (id < 0 || (unsigned)id >= _players.size())
		//	throw_ex(("player id exceeds players count (%d/%d)", id, (int)_players.size()));

		//PlayerSlot &slot = _players[id];
		//if (slot.remote != cid) //client side, no need for this check
		//	throw_ex(("client in connection %d sent wrong channel id %d", cid, id));
		mrt::Serializator s(&message.data);
		deserializeSlots(s);

		float dt = (now + _net_stats.getDelta() - timestamp) / 1000.0f;
		LOG_DEBUG(("respawn, delta: %+d, dt: %g", _net_stats.getDelta(), dt));
	
		World->applyUpdate(s, dt, false);
		} CATCH("on-message(respawn)", throw;);
	break;
	}
	case Message::GameOver: {
		float dt = (now + _net_stats.getDelta() - timestamp) / 1000.0f;
		LOG_DEBUG(("gameover, delta: %+d, dt: %g", _net_stats.getDelta(), dt));

		TRY {
			GameMonitor->gameOver(message.get("area"), message.get("message"), atof(message.get("duration").c_str()) - dt, false);
		} CATCH("on-message(gameover)", throw; )
		break;
	}
	
	case Message::TextMessage: {
		if (message.get("hint") == "1") {
			int id = message.channel;
			if (id < 0 || (unsigned)id >= _players.size())
				throw_ex(("player id exceeds players count (%d/%d)", id, (int)_players.size()));
			PlayerSlot &slot = _players[id];
			slot.displayTooltip(message.get("area"), message.get("message"));			
		} else {
			float dt = (now + _net_stats.getDelta() - timestamp) / 1000.0f;
			LOG_DEBUG(("respawn, delta: %+d, dt: %g", _net_stats.getDelta(), dt));

			TRY {
				GameMonitor->displayMessage(message.get("area"), message.get("message"), atof(message.get("duration").c_str()) - dt);
			} CATCH("on-message(text-message)", throw; )		
		}
		break;
	}
	case Message::DestroyMap: {
		mrt::Serializator s(&message.data);
		int n;
		s.get(n);
		LOG_DEBUG(("%d destroyed map cells", n));
		v3<int> cell;
		while(n--) {
			cell.deserialize(s);
			Map->_destroy(cell.z, v2<int>(cell.x, cell.y));
		}
		break;
	}
	case Message::PlayerMessage: {
		if (_client) {
			Game->getChat()->addMessage(message.get("nick"), message.get("text"));
			break;
		} 
	
		if (_server) {
			int id = message.channel;
			if (id < 0 || (unsigned)id >= _players.size())
				throw_ex(("player id exceeds players count (%d/%d)", id, (int)_players.size()));

			PlayerSlot &slot = _players[id];
			if (slot.remote != cid)
				throw_ex(("client in connection %d sent wrong channel id %d", cid, id));
	
			std::string nick =  _players[id].name;
			Game->getChat()->addMessage(nick, message.get("text"));
			Message msg(message);
			msg.set("nick", nick);
			broadcast(msg, true);
		}
		
		} break;
	
	default:
		LOG_WARN(("unhandled message: %s\n%s", message.getType(), message.data.dump().c_str()));
	};
} CATCH(mrt::formatString("onMessage(%d, %s)", cid, message.getType()).c_str(), { 
	if (_server) 
		_server->disconnect(cid);
	if (_client) {
		_client->disconnect();
		Game->clear();
		GameMonitor->displayMessage("errors", "multiplayer-exception", 1);
	}
});
}

void IPlayerManager::onMap() {
	if (_server == NULL || !_server->active()) {
		LOG_DEBUG(("server is inactive. exists: %s", _server != NULL? "yes": "nope"));
		return;
	}
	LOG_DEBUG(("server is active. restarting players."));
	_server->restart();	
}

void IPlayerManager::ping() {
	if (_client == NULL)
		throw_ex(("ping is possible only in client mode"));
	
	unsigned ts = SDL_GetTicks();
	LOG_DEBUG(("ping timestamp = %u", ts));
	mrt::Serializator s;
	s.add(ts);
	s.add(_net_stats.getDelta());
	
	Message m(Message::Ping);
	s.finalize(m.data);
	_client->send(m);
}


void IPlayerManager::updatePlayers(const float dt) {
	if (_client && !_game_joined)
		return;
	
TRY {

	size_t n = _players.size();

	for(size_t i = 0; i < n; ++i) {
		PlayerSlot &slot = _players[i];
		if (slot.empty())
			continue;
		
		Object *o = slot.getObject();
		if (o != NULL /* && !o->isDead() */) {
			
			//check for Special Zones ;)
			v2<int> p;
			o->getPosition(p);
			v3<int> player_pos(p.x, p.y, o->getZ());
			
			size_t cn = _zones.size();
			for(size_t c = 0; c < cn; ++c) {
				SpecialZone &zone = _zones[c];
				if (!zone.live()) {
					if (_client)
						continue;
				}
				bool in_zone = zone.in(player_pos, zone.live());
				if (in_zone && 
					_global_zones_reached.find(c) == _global_zones_reached.end() && 
					slot.zones_reached.find(c) == slot.zones_reached.end()) {
					
					LOG_DEBUG(("player[%u] zone %u reached.", (unsigned)i, (unsigned)c));
					zone.onEnter(i);
					slot.zones_reached.insert(c);
					if (zone.global())
						_global_zones_reached.insert(c);
					
				} else if (in_zone && zone.live()) {
					zone.onTick(i);
				} else if (!in_zone && zone.live() && slot.zones_reached.find(c) != slot.zones_reached.end()) {
					zone.onExit(i);
					slot.zones_reached.erase(c);
					LOG_DEBUG(("player[%u] has left zone %u", (unsigned)i, (unsigned)c));
				} 
			}
			
			continue;
		}
		
		if (slot.spectator) 
			continue;
		
		slot.dead_time += dt;
		GET_CONFIG_VALUE("engine.player-respawn-interval", float, ri, 0.5f);
		if (slot.dead_time < ri)
			continue;

		if (slot.spawn_limit > 0) {
			--slot.spawn_limit;
			if (slot.spawn_limit <= 0) {
				slot.old_state.clear();
				slot.spectator = true;
				bool over = true;

				for(size_t i = 0; i < _players.size(); ++i) {
					if (!_players[i].empty() && _players[i].spawn_limit > 0) {
						over = false;
						break;
					}
				}

				if (over) {
					GameMonitor->gameOver("messages", "game-over", 5, false);
				}
				continue;
			}
			LOG_DEBUG(("%d lives left", slot.spawn_limit));
		}
				
		LOG_DEBUG(("player in slot %u is dead. respawning. frags: %d", (unsigned)i, slot.frags));

		slot.spawnPlayer(i, slot.classname, slot.animation);

		if (slot.getObject()) {
			Mixer->playSample(slot.getObject(), "respawn.ogg", false);
		}
		
		if (isServer() && slot.remote != -1) {
			Message m(Message::Respawn);
			m.channel = i;
			mrt::Serializator s;
			serializeSlots(s);
			World->generateUpdate(s, false);
			
			s.finalize(m.data);
			send(slot, m);
		}
	}
	
	bool updated = false;

	int spawn_limit;
	Config->get("map.spawn-limit", spawn_limit, 0);

	
	for(size_t i = 0; i < n; ++i) {
		PlayerSlot &slot = _players[i];
		if (slot.spectator) {
			if (slot.control_method != NULL) {
				bool old_fire = slot.old_state.fire != 0;
				slot.updateState(slot.old_state, dt);
				//LOG_DEBUG(("SPECTATOR: %s", slot.old_state.dump().c_str()));
				slot.old_state.get_velocity(slot.map_vel);
				slot.map_vel *= 500;
				slot.map_pos += slot.map_vel * dt;

				if (_client && !old_fire && slot.old_state.fire != 0) {
					LOG_DEBUG(("requesting extra life..."));
					slot.need_sync = true;
					updated = true;
					continue;
				}
			}

			if (!_client && slot.old_state.fire) {
				LOG_DEBUG(("respawn button hit!"));
				if (spawn_limit <= 0) {
					//infinite lives
					slot.spectator = false;
				} else { 
					int max = 1;
					int max_slot = -1;
				
					for(size_t j = 0; j < n; ++j) {
						if (i == j)
							continue;
						PlayerSlot &slot = _players[j];
						if (!slot.empty() && slot.spawn_limit > max) {
							max = slot.spawn_limit;
							max_slot = j;
						}
					}
					if (max_slot >= 0) {
						slot.spawn_limit = 2;
						slot.spectator = false;
						--_players[max_slot].spawn_limit;
					}
				}
			}
			continue;
		}

		Object *obj = slot.getObject();
		if (obj != NULL) {
			if (slot.control_method != NULL) {
				PlayerState state = obj->getPlayerState();
				bool hint = state.hint_control;
				slot.updateState(state, dt);
				
				obj->updatePlayerState(state);
				if (state.hint_control && !hint) {
					slot.displayLast();
				}
			}
		
			if (obj->getPlayerState() != slot.old_state) {
				slot.old_state = obj->getPlayerState();
				slot.need_sync = true;
			}
		}
		
		if (slot.need_sync) //keep it here, another code may trigger need_sync
			updated = true;	
	}
	
	if (!_object_states.empty())
		updated = true;
			
	if (_client && _players.size() != 0 && updated) {
		mrt::Serializator s;

		for(size_t i = 0; i < n; ++i) {
			PlayerSlot &slot = _players[i];
			if (slot.remote == -1 || !slot.need_sync)
				continue;
			
			if (!slot.spectator) {
				const Object * o = slot.getObject();
				if (o == NULL)
					continue;
				
				o->getPlayerState().serialize(s);
			} else {
				slot.old_state.serialize(s);
			}
	
			Message m(Message::PlayerState);
			m.channel = i;
			s.finalize(m.data);
			_client->send(m);
			_players[i].need_sync = false;
		}
	}
	//cross-players state exchange


	if (_server && updated) {
		bool send = false;
		mrt::Serializator s;

		for(size_t j = 0; j < n; ++j) {

			PlayerSlot &slot = _players[j];
			if (!slot.empty() && slot.need_sync) {
				//LOG_DEBUG(("object in slot %d: %s (%d) need sync [%s]", 
				//	j, slot.getObject()->animation.c_str(), slot.getObject()->getID(), slot.getObject()->getPlayerState().dump().c_str()));
				const Object * o = slot.getObject();
				if (o == NULL)
					continue;
				
				s.add(slot.id);
				s.add(o->getPlayerState());
				World->serializeObjectPV(s, o);
				s.add(slot.dont_interpolate);
				
				send = true;
				slot.need_sync = false;
				slot.dont_interpolate = false;
			}
		}
		if (!_object_states.empty()) {
			for(ObjectStates::const_iterator i = _object_states.begin(); i != _object_states.end(); ++i) {
				const int id = *i;
				const Object * o = World->getObjectByID(id);
				if (o == NULL)
					continue;
			
				s.add(id);
				s.add(o->getPlayerState());
				World->serializeObjectPV(s, o);
				s.add(false);
				
				send = true;
			}
			_object_states.clear();			
		}

		if (send) {
			Message m(Message::UpdatePlayers);
			s.finalize(m.data);
			broadcast(m, true);
		}
	}
} CATCH("updatePlayers", {
		Game->clear();
		GameMonitor->displayMessage("errors", "multiplayer-exception", 1);
})
}

IPlayerManager::IPlayerManager() : 
	_server(NULL), _client(NULL), _players(), _next_ping(0), _ping(false), _next_sync(true), _game_joined(false)
{
	on_destroy_map_slot.assign(this, &IPlayerManager::onDestroyMap, Map->destroyed_cells_signal);
	on_load_map_slot.assign(this, &IPlayerManager::onMap, Map->load_map_final_signal);
	on_object_death_slot.assign(this, &IPlayerManager::onPlayerDeath, World->on_object_death);
}

IPlayerManager::~IPlayerManager() {}

void IPlayerManager::startServer() {
	clear();
	TRY {
		if (_client != NULL) {
			delete _client;
			_client = NULL;
			_recent_address.clear();
		}
		if (_server == NULL) {
			_server = new Server;
			_server->init();
		}
	} CATCH("server initialization", {
		if (_server != NULL) {
			delete _server;
			_server = NULL;
		}
	});
}

void IPlayerManager::startClient(const std::string &address, const size_t n) {
	clear();
	if (_server != NULL) {
		delete _server;
		_server = NULL;
	}
	//if (_client != NULL && _recent_address == address) {
	//	LOG_DEBUG(("skipping connection to the same address (%s)", address.c_str()));
	//	return;
	//}
	delete _client;
	_client = NULL;
	
	_local_clients = n;
	World->setSafeMode(true);

	TRY {
		_client = new Client;
		_client->init(address);
	} CATCH("_client.init", { 
		delete _client; _client = NULL; 
		Game->clear();
		GameMonitor->displayMessage("errors", "multiplayer-exception", 1);
		return;
	});
	_recent_address = address;
}
void IPlayerManager::clear() {
	LOG_DEBUG(("deleting server/client if exists."));
	_ping = false;
	_game_joined = false;
	//delete _server; _server = NULL;
	//delete _client; _client = NULL;
	//_local_clients = 0;
	_net_stats.clear();

	GET_CONFIG_VALUE("multiplayer.sync-interval", float, sync_interval, 103.0/101);
	_next_sync.set(sync_interval);

	LOG_DEBUG(("cleaning up players..."));
	_global_zones_reached.clear();
	_players.clear();	
	_zones.clear();
	_object_states.clear();
}

void IPlayerManager::addSlot(const v3<int> &position) {
	PlayerSlot slot;
	slot.position = position;
	_players.push_back(slot);
}

void IPlayerManager::addSpecialZone(const SpecialZone &zone) {
	if (zone.size.x == 0 || zone.size.y == 0)
		throw_ex(("zone size cannot be 0"));
	LOG_DEBUG(("adding zone '%s' named '%s' at %d %d (%dx%d)", zone.type.c_str(), zone.name.c_str(), zone.position.x, zone.position.y, zone.size.x, zone.size.y));
	_zones.push_back(zone);
}

PlayerSlot &IPlayerManager::getSlot(const unsigned int idx) {
	if (idx >= _players.size())
		throw_ex(("slot #%u does not exist", idx));
	return _players[idx];
}
const PlayerSlot &IPlayerManager::getSlot(const unsigned int idx) const {
	if (idx >= _players.size())
		throw_ex(("slot #%u does not exist", idx));
	return _players[idx];
}

const int IPlayerManager::getSlotID(const int object_id) const {
	if (object_id <= 0)
		return -1;

	for(int i = 0; i != (int)_players.size(); ++i) {
		if (_players[i].id == object_id) 
			return i;
	}
	return -1;
}

PlayerSlot *IPlayerManager::getSlotByID(const int id) {
	if (id <= 0)
		return NULL;
	
	for(std::vector<PlayerSlot>::iterator i = _players.begin(); i != _players.end(); ++i) {
		PlayerSlot &slot = *i;
		if (slot.id == id) 
			return &*i;
	}
	return NULL;
}
const PlayerSlot *IPlayerManager::getSlotByID(const int id) const {
	for(std::vector<PlayerSlot>::const_iterator i = _players.begin(); i != _players.end(); ++i) {
		const PlayerSlot &slot = *i;
		if (slot.id == id) 
			return &*i;
	}
	return NULL;
}


const int IPlayerManager::findEmptySlot() const {
	int i, n = _players.size();
	for(i = 0; i < n; ++i) {
		if (_players[i].empty() && _players[i].remote == -1)
			break;
	}
	if (i == n) 
		throw_ex(("no available slots found from %d", n));
	return i;
}

const int IPlayerManager::spawnPlayer(const std::string &classname, const std::string &animation, const std::string &control_method) {
	int i = findEmptySlot();
	PlayerSlot &slot = _players[i];

	slot.createControlMethod(control_method);
	
	LOG_DEBUG(("player[%d]: %s.%s using control method: %s", i, classname.c_str(), animation.c_str(), control_method.c_str()));
	slot.spawnPlayer(i, classname, animation);
	return i;
}

void IPlayerManager::validateViewports() {
		if (Map->loaded()) {
			for(size_t p = 0; p < _players.size(); ++p) {
				PlayerSlot &slot = _players[p];
				if (!slot.visible) 
					continue;
				
				slot.validatePosition(slot.map_pos);				
			}
		}
}

void IPlayerManager::tick(const float dt) {
	if (_server != NULL && (!Map->loaded() || _players.empty()))
		return;
TRY {
	Uint32 now = SDL_GetTicks();
	if (_server) {
		if (_next_sync.tick(dt) && isServerActive()) {
			Message m(Message::UpdateWorld);
			{
				mrt::Serializator s;
				serializeSlots(s);
				World->generateUpdate(s, true);
				GameMonitor->serialize(s);
				s.finalize(m.data);
			}
			LOG_DEBUG(("sending world update... (size: %u)", (unsigned)m.data.getSize()));
			broadcast(m, true);
		}
		_server->tick(dt);
	}

	if (_client) {
		_client->tick(dt);
		if (_ping && now >= _next_ping) {
			ping();
			GET_CONFIG_VALUE("multiplayer.ping-interval", int, ping_interval, 1500);
			_next_ping = (int)(now + ping_interval); //fixme: hardcoded value
		}
	}
} CATCH("tick", {
		Game->clear();
		GameMonitor->displayMessage("errors", "multiplayer-exception", 1);
		return;
})
	//bool listener_set = false;
	v2<float> listener_pos, listener_vel, listener_size;
	float listeners = 0;
	for(size_t pi = 0; pi < _players.size(); ++pi) {
		PlayerSlot &slot = _players[pi];
		if (!slot.visible)
			continue;
		const Object * o = slot.getObject();
		if (o == NULL)
			continue;

		v2<float> pos, vel;
		o->getInfo(pos, vel);
		listener_pos += pos;
		listener_vel += vel;
		listeners += 1;
		listener_size += o->size;
	}

	if (listeners > 0) {
		listener_pos /= listeners;
		listener_vel /= listeners;
		listener_size /= listeners;
		Mixer->setListener(listener_pos.convert2v3(0), listener_vel.convert2v3(0), listener_size.length());
	}

	for(size_t pi = 0; pi < _players.size(); ++pi) {
		PlayerSlot &slot = _players[pi];
		slot.tick(dt);
	}

	validateViewports();
}


void IPlayerManager::render(sdlx::Surface &window, const int vx, const int vy) {
		size_t local_idx = 0;
		for(size_t p = 0; p < _players.size(); ++p) {
			PlayerSlot &slot = _players[p];
			if (!slot.visible)
				continue;
			++local_idx;
			
			if (slot.viewport.w == 0) {
				assert(local_idx > 0);
				
				if (local_idx > _local_clients || _local_clients > 2)
					throw_ex(("this client cannot handle %u clients(local index: %u)", (unsigned)_local_clients, (unsigned) local_idx));
				
				if (_local_clients == 1) {
					slot.viewport = window.getSize();
				} else if (_local_clients == 2) {
					slot.viewport = window.getSize();
					slot.viewport.w /= 2;
					if (local_idx == 2) {
						slot.viewport.x += slot.viewport.w;
					}
				}
			}
				
			slot.render(window, vx, vy);
		
			GET_CONFIG_VALUE("engine.show-special-zones", bool, ssz, false);

			if (ssz) {
				for(size_t i = 0; i < _zones.size(); ++i) {
					sdlx::Rect pos(_zones[i].position.x, _zones[i].position.y, _zones[i].size.x, _zones[i].size.y);
					static sdlx::Surface zone;
					if (zone.isNull()) {
						//zone.createRGB(_zones[i].size.x, _zones[i].size.y, 32); 
						zone.createRGB(32, 32, 32); 
						zone.convertAlpha();
						zone.fill(zone.mapRGBA(255, 0, 0, 51));
					}

					pos.x -= (int)slot.map_pos.x;
					pos.y -= (int)slot.map_pos.y;
					for(int y = 0; y <= (_zones[i].size.y - 1) / zone.getHeight(); ++y) 
						for(int x = 0; x <= (_zones[i].size.x - 1) / zone.getWidth(); ++x) 
							window.copyFrom(zone, pos.x + x * zone.getWidth(), pos.y + y * zone.getHeight());
				}
			}
		}
}

void IPlayerManager::screen2world(v2<float> &pos, const int p, const int x, const int y) {
	PlayerSlot &slot = _players[p];
	pos.x = slot.map_pos.x + x;
	pos.y = slot.map_pos.x + y;
}

const size_t IPlayerManager::getSlotsCount() const {
	return _players.size();
}

const size_t IPlayerManager::getFreeSlotsCount() const {
	size_t c = 0, n = _players.size();
	for(size_t i = 0; i < n; ++i) {
		if (_players[i].empty() && _players[i].remote == -1)
			++c;
	}
	return c;
}


void IPlayerManager::serializeSlots(mrt::Serializator &s) const {
	s.add(_players);
	s.add(_global_zones_reached);
}

void IPlayerManager::deserializeSlots(const mrt::Serializator &s) {
	s.get(_players);
	s.get(_global_zones_reached);
}

void IPlayerManager::broadcast(const Message &_m, const bool per_connection) {
	assert(_server != NULL);
	
	size_t n = _players.size();
	if (per_connection) {
		std::set<int> seen;
		for(size_t i = 0; i < n; ++i) {
			const PlayerSlot &slot = _players[i];
			
			if (slot.remote == -1 || seen.find(slot.remote) != seen.end())
				continue;
			seen.insert(slot.remote);
			_server->send(slot.remote, _m);
		}
	} else {
		Message m(_m);
		for(size_t i = 0; i < n; ++i) {
			const PlayerSlot &slot = _players[i];
			if (slot.remote == -1 || slot.empty())
				continue;
	
			m.channel = i;
			_server->send(slot.remote, m);
		}
	}
}

void IPlayerManager::send(const PlayerSlot &slot, const Message & msg) {
	if (_server == NULL)
		throw_ex(("PlayerManager->send() allowed only in server mode"));
	int cid = slot.remote;
	if (cid != -1)
		_server->send(cid, msg);
}


const bool IPlayerManager::isServerActive() const {
	if (_server == NULL || !_server->active())
		return false;
	
	int n = _players.size();
	for(int i = 0; i < n; ++i) {
		const PlayerSlot &slot = _players[i];
		if (slot.remote != -1 && !slot.empty())
			return true;
	}
	return false;
}

#include "special_owners.h"

void IPlayerManager::onPlayerDeath(const Object *player, const Object *killer) {
	if (player == NULL || 
		killer == NULL || 
		_client != NULL || 
		killer->getSlot() < 0 || 
		killer->getSlot() >= (int)_players.size() || 
		GameMonitor->gameOver() ||
		RTConfig->game_type == GameTypeCTF)
		return;
		
	//LOG_DEBUG(("handler %s %s", player->animation.c_str(), killer->animation.c_str()));

	if (RTConfig->game_type != GameTypeCooperative) { //skip this check in coop mode
		PlayerSlot *player_slot = getSlotByID(player->getID());
		if (player_slot == NULL)
			return;
	} else {
		if (player->hasOwner(OWNER_COOPERATIVE) || player->getSlot() >= 0) {
			return;
		}
	}
	//LOG_DEBUG(("prepare: object %s killed by %s", player->animation.c_str(), killer->animation.c_str()));
	PlayerSlot &slot = _players[killer->getSlot()];

	LOG_DEBUG(("object %s killed by %s", player->animation.c_str(), killer->animation.c_str()));
		
	if (slot.id == player->getID()) { //suicide
		if (slot.frags > 0)
			--(slot.frags);
	} else {
		++(slot.frags);
	}
}

void IPlayerManager::gameOver(const std::string &area, const std::string &message, const float time) {
	if (!isServerActive())
		return;
	Message m(Message::GameOver);
	m.set("area", area);
	m.set("message", message);
	m.set("duration", mrt::formatString("%g", time));
	broadcast(m, true);
}

void IPlayerManager::onDestroyMap(const std::set<v3<int> > & cells) {
	if (!_server)
		return;
		
	mrt::Serializator s;
	s.add((int)cells.size());
	for(std::set<v3<int> >::const_iterator i = cells.begin(); i != cells.end(); ++i) {
		i->serialize(s);
	}

	Message m(Message::DestroyMap);
	s.finalize(m.data);
	broadcast(m, true);	
}

void IPlayerManager::updateControls() {
	int n = _players.size();
	int pn = 0;
	int p1 = -1, p2 = -1;
	
	for(int i = 0; i < n; ++i) {
		const PlayerSlot &slot = _players[i];
		if (slot.visible) {
			++pn;
			if (p1 == -1) {
				p1 = i;
				continue;
			}
			if (p2 == -1) {
				p2 = i;
				continue;
			}
		}
	}
	//LOG_DEBUG(("p1: %d, p2: %d", p1, p2));
	std::string cm1, cm2;
	switch(pn) {
	case 2: 
		Config->get("player.control-method-1", cm1, "keys-1");	
		Config->get("player.control-method-2", cm2, "keys-2");	
		_players[p1].createControlMethod(cm1);
		_players[p2].createControlMethod(cm2);
	break;
	case 1: 
		Config->get("player.control-method", cm1, "keys");	
		_players[p1].createControlMethod(cm1);
	break;	
	}
}

PlayerSlot *IPlayerManager::getMySlot() {
	for(size_t i = 0; i < _players.size(); ++i) {
		if (_server && _players[i].remote == -1 && !_players[i].empty()) 
			return &_players[i];

		if (_client && _players[i].remote != -1 && !_players[i].empty()) 
			return &_players[i];

	}
	return NULL;
}

void IPlayerManager::broadcastMessage(const std::string &area, const std::string &message, const float duration) {
TRY {
	Message m(Message::TextMessage);
	m.set("area", area);
	m.set("message", message);
	m.set("duration", mrt::formatString("%g", duration));
	m.set("hint", "0");
	broadcast(m, true);	
} CATCH("say", {
		Game->clear();
		GameMonitor->displayMessage("errors", "multiplayer-exception", 1);
})

}

void IPlayerManager::sendHint(const int slot_id, const std::string &area, const std::string &message) {
	PlayerSlot &slot = getSlot(slot_id);
	
	Message m(Message::TextMessage);
	m.channel = slot_id;
	m.set("area", area);
	m.set("message", message);
	m.set("hint", "1");
	send(slot, m);
}

void IPlayerManager::say(const std::string &message) {
TRY {
	LOG_DEBUG(("say('%s')", message.c_str()));

	Message m(Message::PlayerMessage);
	m.set("text", message);
	
	if (_server) {
		PlayerSlot *my_slot = NULL;
		for(size_t i = 0; i < _players.size(); ++i) {
			if (_players[i].visible) {
				my_slot = &_players[i];
				break;
			}
		}
		
		if (my_slot == NULL) 
			throw_ex(("cannot get my slot."));
		
		m.set("nick", my_slot->name);
		Game->getChat()->addMessage(my_slot->name, message);
		broadcast(m, true);
	}
	if (_client) {
		size_t i;
		for(i = 0; i < _players.size(); ++i) {
			if (_players[i].visible) 
				break;
		}
		if (i == _players.size())
			throw_ex(("cannot get my slot"));
		
		m.channel = i;
		_client->send(m);
	}
} CATCH("say", {
		Game->clear();
		GameMonitor->displayMessage("errors", "multiplayer-exception", 1);
})
}

const SpecialZone& IPlayerManager::getNextCheckpoint(PlayerSlot &slot) {
	bool final = false;
	do {
		for(size_t i = 0; i < _zones.size(); ++i) {
			const SpecialZone &zone = _zones[i];
			if (zone.type != "checkpoint" || _global_zones_reached.find(i) != _global_zones_reached.end() ||
				slot.zones_reached.find(i) != slot.zones_reached.end()) 
					continue;
			return zone;
		}

		if (final)
			throw_ex(("cannot release any checkpoints"));

		//clear checkpoint list
		LOG_DEBUG(("all checkpoints reached. cleaning up..."));
		size_t last = 0;
		for(size_t i = 0; i < _zones.size(); ++i) {
			const SpecialZone &zone = _zones[i];
			if (zone.type == "checkpoint") {
				last = i;
				slot.zones_reached.erase(i);
			}
		}
		slot.zones_reached.insert(last);
		final = true;
	} while(true);
}

void IPlayerManager::fixCheckpoints(PlayerSlot &slot, const SpecialZone &zone) {
	for(size_t i = 0; i < _zones.size(); ++i) {
		const SpecialZone &zone = _zones[i];
		if (zone.type == "checkpoint")
			slot.zones_reached.erase(i);
	}
	for(size_t i = 0; i < _zones.size(); ++i) {
		const SpecialZone &z = _zones[i];
		if (zone.type == "checkpoint")
			slot.zones_reached.insert(i);
		if (z.name == zone.name)
			return;
	}
}

void IPlayerManager::sendObjectState(const int id, const PlayerState & state) {
	if (!isServerActive() || getSlotByID(id) != NULL) //object doesnt reside in any slot.
		return;
	_object_states.insert(id);
}


void IPlayerManager::requestObjects(const int first_id) {
	if (_client == NULL)
		return;
	Message m(Message::RequestObjects);
	mrt::Serializator s;
	s.add(first_id);
	s.finalize(m.data);
	_client->send(m);
}

void IPlayerManager::disconnect_all() {
	if (_server == NULL) 
		return;
	LOG_DEBUG(("disconnecting all clients"));
	_server->disconnect_all();
}
