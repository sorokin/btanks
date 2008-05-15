#ifndef __BTANKS_PROTOCOL_H__
#define __BTANKS_PROTOCOL_H__

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


#include <sys/types.h>
#include <map>
#include <string>
#include "mrt/serializable.h"
#include "mrt/chunk.h"

namespace mrt {
	class TCPSocket;
}

	
class Message : public mrt::Serializable {
public: 
	enum Type {
		None, Ping, Pang, Pong,
		ServerStatus,
		RequestPlayer,
		GameJoined,
		PlayerState,
		UpdatePlayers,
		UpdateWorld, 
		Respawn, 
		GameOver,
		TextMessage, 
		DestroyMap, 
		PlayerMessage, 
		RequestObjects,
		JoinTeam,
		ServerDiscovery
	};
	
	Message();
	Message(const Type type);
	
	const char * getType() const;
	inline const bool realtime() const {
		return type == Ping || type == Pong || type == Pang || type == PlayerState || type == UpdatePlayers;
	}

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	void set(const std::string &key, const std::string &value);
	const bool has(const std::string &key) const;
	const std::string &get(const std::string &key) const;
	
	int channel;
	Type type;

	mrt::Chunk data;
	
	inline unsigned get_timestamp() const { return timestamp; }
	
private:
	typedef std::map<const std::string, std::string> AttrMap;
	AttrMap _attrs;
	unsigned timestamp;
};

#endif

