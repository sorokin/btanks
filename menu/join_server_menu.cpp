
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
#include "join_server_menu.h"
#include "button.h"
#include "mrt/logger.h"
#include "menu.h"
#include "menu_config.h"
#include "host_list.h"
#include "map_details.h"
#include "prompt.h"
#include "text_control.h"
#include "player_manager.h"
#include "game.h"
#include "game_monitor.h"
#include "chooser.h"
#include "config.h"
#include "i18n.h"
#include "upper_box.h"
#include "net/scanner.h"
#include "label.h"

JoinServerMenu::JoinServerMenu(MainMenu *parent, const int w, const int h) : _parent(parent), _scanner(NULL) {
	_back = new Button("big", I18n->get("menu", "back"));
	_add = new Button("medium_dark",  I18n->get("menu", "add"));
	_del = new Button("medium_dark",  I18n->get("menu", "delete"));
	_scan = new Button("big", I18n->get("menu", "scan"));
	_join = new Button("big", I18n->get("menu", "join"));
	_upper_box = new UpperBox(w - 48, 80, false);
	_add_dialog = new Prompt(w / 2, 96, new HostTextControl("medium"));

	const int host_list_w = 2 * (w - 64)/3;

	int bw, bh, xp = 48;

	_add->getSize(bw, bh);
	add(16, h - 80 - bh, _add);

	_del->getSize(bw, bh);
	add(xp + host_list_w - 32 - bw, h - 80 - bh, _del);

	_back->getSize(bw, bh);
	add(xp, h - 16 - bh, _back);
	xp += 16 + bw;
	

#ifndef RELEASE
	_scan->getSize(bw, bh);
	add(xp, h - 16 - bh, _scan);
#endif
	
	_join->getSize(bw, bh);
	add(w - 64 - bw, h - 16 - bh, _join);

	sdlx::Rect list_pos(16, 128, host_list_w, h - 256);

	_hosts = new HostList("multiplayer.recent-hosts", list_pos.w, list_pos.h);
	add(list_pos.x, list_pos.y, _hosts);
	
	_upper_box->getSize(bw, bh);
	add((w - bw) / 2 - 8, 32, _upper_box);

	sdlx::Rect map_pos(list_pos.x + list_pos.w + 16, 128, (w - 64) / 3, h - 256);

	_details = new MapDetails(map_pos.w, map_pos.h, false);
	add(map_pos.x, map_pos.y, _details);	
	
	_add_dialog->getSize(bw, bh);
	add(w / 3, (h - bh) / 2, _add_dialog);
	_add_dialog->hide();
	
	//client vehicle stub.
	std::vector<std::string> options;

	options.clear();
	options.push_back("?");
	options.push_back("launcher");
	options.push_back("shilka");
	options.push_back("tank");
	options.push_back("machinegunner");
	options.push_back("civilian");
	options.push_back("mortar");
		
	_vehicle = new Chooser("medium", options, "menu/vehicles.png");
	_vehicle2 = new Chooser("medium", options, "menu/vehicles.png");

	_vehicle->disable(0);
	_vehicle2->disable(0);

	for(int i = 4; i < _vehicle->size(); ++i) {
		_vehicle->disable(i);
		_vehicle2->disable(i);
	}

	TRY {
		std::string def_v;

		Config->get("menu.default-vehicle-1", def_v, "tank");
		_vehicle->set(def_v);

		Config->get("menu.default-vehicle-2", def_v, "tank");
		_vehicle2->set(def_v);
	} CATCH("_vehicle->set()", {})

	_vehicle->getSize(bw, bh);
		
	add(map_pos.x + map_pos.w / 2 - bw / 2, map_pos.y + map_pos.h - bh * 2, _vehicle);
	add(map_pos.x + map_pos.w / 2 + bw / 2, map_pos.y + map_pos.h - bh * 2, _vehicle2);
}

void JoinServerMenu::join() {
	LOG_DEBUG(("join()"));
	if (_hosts->empty()) {
		LOG_DEBUG(("please add at least one host in list."));
		return;
	}
	
	std::string host = _hosts->getValue();
	_hosts->promote(_hosts->get());

	Config->set("menu.default-vehicle-1", _vehicle->getValue());
	
	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);
		
	Game->clear();
	PlayerManager->startClient(host, split?2:1);
}

void JoinServerMenu::tick(const float dt) {
	Container::tick(dt);

	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);

	if (split && _vehicle2->hidden())
		_vehicle2->hide(false);
	if (!split && !_vehicle2->hidden())
		_vehicle2->hide();

	if (_vehicle->changed()) {
		_vehicle->reset();
		Config->set("menu.default-vehicle-1", _vehicle->getValue());
	}

	if (_vehicle2->changed()) {
		_vehicle2->reset();
		Config->set("menu.default-vehicle-2", _vehicle2->getValue());
	}
	
	if (_back->changed()) {
		LOG_DEBUG(("[back] clicked"));
		onHide();
		MenuConfig->save();
		_back->reset();
		_parent->back();
	}
	if (_add->changed()) {
		_add->reset();
		_add_dialog->hide(false);
	}
	
	if (_del->changed()) {
		_del->reset();
		_hosts->remove(_hosts->get());
	}
	
	if (_add_dialog->changed()) {
		_add_dialog->reset();
		_add_dialog->hide();
		if (!_add_dialog->get().empty())
			_hosts->append(_add_dialog->get());
		
		_add_dialog->set(std::string());
	}


	if (_scan->changed()) {
		_scan->reset();
		if (_scanner == NULL) {
			_scanner = new Scanner;
		}
		_scanner->scan();
	}
	
	if (_join->changed()) {
		_join->reset();
		join();
	}
	
	if (_scanner != NULL && _scanner->changed()) {
		_scanner->reset();
		std::set<std::string> hosts;
		_scanner->get(hosts);
/*		int n = _hosts->size();
		for(int i = 0; i < n; ++i) {
			const Label * label = dynamic_cast<const Label*>(_hosts->getItem(i));
			if (label == NULL) 
				continue;
			
			std::set<std::string>::iterator h = hosts.find(label->get());
			if (h != hosts.end())
				hosts.erase(h);
		}
		*/
			
		for(std::set<std::string>::iterator i = hosts.begin(); i != hosts.end(); ++i) {
			_hosts->append(*i);
		}
	}
}

void JoinServerMenu::onHide() {
	if (_scanner != NULL) {
		delete _scanner;
		_scanner = NULL;
	}
}


bool JoinServerMenu::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
	
	switch(sym.sym) {

	case SDLK_RETURN:
		join();
		return true;
	
	case SDLK_a: 
		_add_dialog->hide(false);
		return true;
	
	case SDLK_ESCAPE: 
		onHide();
		MenuConfig->save();
		_parent->back();
		return true;
	default: ;
	}
	return false;
}

JoinServerMenu::~JoinServerMenu() {}
