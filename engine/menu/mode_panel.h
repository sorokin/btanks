#ifndef BTANKS_MENU_MODE_PANEL_H__
#define BTANKS_MENU_MODE_PANEL_H__

#include "container.h"

class Box;
struct MapDesc;
class Chooser;
class Checkbox;
class Label;

class ModePanel : public Container {
public: 
	ModePanel(const int w);

	void tick(const float dt);
	void set(const MapDesc &map, const int mode);

private: 
	void validate();
	Box *_background;

	typedef std::map<int, std::string> TimeLimits;
	TimeLimits _time_limits;

	Chooser *_time_limit, *_teams;
	Checkbox * _random_respawn;
	Label *_tl_label, *_rr_label, *_teams_label;
	int mode;
};

#endif

