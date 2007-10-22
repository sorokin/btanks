#ifndef BTANKS_AI_WAYPOINTS_H__
#define BTANKS_AI_WAYPOINTS_H__

#include <string>
//#include <set>
#include "alarm.h"
#include "export_btanks.h"

class Object;
namespace mrt {
	class Serializator;
}

namespace ai {

class BTANKSAPI Waypoints {
public: 
	Waypoints();
	virtual void onSpawn(const Object *object);
	void calculate(Object *object, const float dt);

	virtual void onObstacle(const Object *o) = 0;

	virtual ~Waypoints() {}

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	const bool active() const;
protected:
	//std::set<std::string> obstacle_filter;
	bool _avoid_obstacles;
	bool _stop_on_obstacle;
	
private: 
	Alarm _reaction_time;
	bool _stop;
	std::string _waypoint_name;
};

}

#endif

