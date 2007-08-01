#ifndef BTANKS_JOIN_SERVER_MENU_H__
#define BTANKS_JOIN_SERVER_MENU_H__

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

#include "sdlx/rect.h"
#include "base_menu.h"

class Button;
class MainMenu;
class HostList;
class MapDetails;
class Prompt;
class Chooser;
class UpperBox;

class JoinServerMenu : public BaseMenu {
public:
	JoinServerMenu(MainMenu *parent, const int w, const int h);
	~JoinServerMenu();

	void tick(const float dt);
	void join();
	void remove();
	
	virtual bool onKey(const SDL_keysym sym);

private: 
	MainMenu *_parent;
	UpperBox *_upper_box;
	HostList *_hosts;
	MapDetails *_details;
	Prompt *_add_dialog;
	Chooser *_vehicle;
	Button *_back, *_add, *_del, *_scan, *_join;
};

#endif

