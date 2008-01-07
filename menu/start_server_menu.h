#ifndef BTANKS_START_SERVER_MENU_H__
#define BTANKS_START_SERVER_MENU_H__

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

#include "sdlx/rect.h"
#include "base_menu.h"

class MapPicker;
class Button;
class MainMenu;

class StartServerMenu : public BaseMenu {
public:
	StartServerMenu(MainMenu *parent, const int w, const int h);
	~StartServerMenu();

	void start();	
	void tick(const float dt);
	virtual bool onKey(const SDL_keysym sym);
	
private: 
	MainMenu *_parent;
	MapPicker *_map_picker;
	Button *_back, *_start;
};

#endif

