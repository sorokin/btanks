#ifndef __BTANKS_PLAYER_MANAGER_H__
#define __BTANKS_PLAYER_MANAGER_H__

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

#include "export_btanks.h"
#include "mrt/singleton.h"
#include <vector>
#include <string>
#include <set>
#include "math/v2.h"
#include "math/v3.h"
#include "alarm.h"
#include "zbox.h"
#include "netstats.h"

//could be forward, but incompatible with stlport
#include "special_zone.h"
#include "player_slot.h"
#include "sl08/sl08.h"

namespace mrt {
class Chunk;
}

namespace sdlx {
class Rect;
class Surface;
}

class PlayerState;
class Server;
class Client;
class Message;
class Object;

class BTANKSAPI IPlayerManager {
public:
	IPlayerManager();
	DECLARE_SINGLETON(IPlayerManager);
	~IPlayerManager();
		
	void startServer();
	void startClient(const std::string &address, const size_t n);
	void clear();
	
	inline const bool isClient() const { return _client != NULL; }
	inline const bool isServer() const { return _server != NULL; }	
	const bool isServerActive() const;

	void addSlot(const v3<int> &position);
	void addSpecialZone(const SpecialZone& zone);

	PlayerSlot *getMySlot(); //remove me

	PlayerSlot &getSlot(const unsigned int idx);
	const PlayerSlot &getSlot(const unsigned int idx) const;

	const int getSlotID(const int object_id) const;
	PlayerSlot *getSlotByID(const int id);
	const PlayerSlot *getSlotByID(const int id) const;

	const size_t getSlotsCount() const;
	const size_t getFreeSlotsCount() const;
	
	void screen2world(v2<float> &pos, const int p, const int x, const int y);

	void getDefaultVehicle(std::string &vehicle, std::string &animation);
	
	const int findEmptySlot() const;
	const int spawnPlayer(const std::string &classname, const std::string &animation, const std::string &method);

	void updatePlayers(const float dt);
	void ping();
	
	void tick(const float dt);
	void render(sdlx::Surface &window, const int x, const int y);
	
	const int onConnect(Message &message);
	void onMessage(const int id, const Message &message, const int delta);
	void onDisconnect(const int id);	
	void onMap();
	
	void onPlayerDeath(const Object *player, const Object *killer);
	void gameOver(const std::string &reason, const float time);
	
	void onDestroyMap(const std::set<v3<int> > & cells);
	
	void validateViewports();
	
	//for special zones
	void send(const PlayerSlot &slot, const Message & msg);
	void say(const std::string &message);
	
	void broadcastMessage(const std::string &area, const std::string &message, const float duration);
	void sendHint(const int slot_id, const std::string &area, const std::string &message);
	
	void updateControls(); //recreates control methods.
	
	const SpecialZone& getNextCheckpoint(PlayerSlot &slot); 
	void fixCheckpoints(PlayerSlot &slot, const SpecialZone &zone); 
	
	void sendObjectState(const int id, const PlayerState & state);
	void requestObjects(const int first_id);

private: 
	sl08::slot1<void, const std::set<v3<int> > &, IPlayerManager> on_destroy_map_slot;
	sl08::slot0<void, IPlayerManager> on_load_map_slot;
	
	void serializeSlots(mrt::Serializator &s) const;
	void deserializeSlots(const mrt::Serializator &s);
	
	void broadcast(const Message &m, const bool per_connection);
	

	IPlayerManager(const IPlayerManager &);
	const IPlayerManager& operator=(const IPlayerManager &);

	Server *_server;
	Client *_client;

	size_t _local_clients;

	std::set<int> _global_zones_reached;
	std::vector<PlayerSlot> _players;
	std::vector<SpecialZone> _zones;

	NetStats _net_stats;
	unsigned _next_ping;
	bool _ping;
	Alarm _next_sync;
	bool _game_joined;
	std::string _recent_address;
	
	typedef std::set<int> ObjectStates;
	ObjectStates _object_states;
};

SINGLETON(BTANKSAPI, PlayerManager, IPlayerManager);

#endif
	
