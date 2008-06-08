
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
#include "host_list.h"
#include "config.h"
#include "host_item.h"

HostList::HostList(const std::string &config_key, const int w, const int h) : 
ScrollList("menu/background_box.png", "medium_dark", w, h), _config_key(config_key) {
	std::string str_hosts;
	Config->get(config_key, str_hosts, std::string());
	std::vector<std::string> hosts;
	mrt::split(hosts, str_hosts, " ");
	for(size_t i = 0; i < hosts.size(); ++i) {
		if (hosts[i].empty())
			continue;
		
		//mrt::to_lower(hosts[i]);
		//ScrollList::append(hosts[i]);
		append(hosts[i]);
	}
}

void HostList::promote() {
	size_t i = get();
	
	List::iterator li = _list.begin();
	
	for(size_t n = i; n--; ++li);
	Control * host = *li;
	_list.erase(li);	
	_list.push_front(host);
	_current_item = 0;
}

void HostList::append(const std::string &_item) {
	std::string item = _item;
	mrt::to_lower(item);
	int a;
	bool has_ip = (sscanf(item.c_str(), "%d.%d.%d.%d", &a, &a, &a, &a) == 4);
	
	for(List::iterator i = _list.begin(); i != _list.end(); ++i) {
		HostItem *l = dynamic_cast<HostItem *>(*i);
		if (l == NULL) 
			continue;
		
		if (item == l->ip || item == l->name)
			return;
	}

	HostItem *new_item = new HostItem();
	size_t sp = item.find('/');

	if (sp != std::string::npos) {
		new_item->name = item.substr(sp + 1);
		new_item->ip = item.substr(0, sp);
	} else {
		if (has_ip) {
			new_item->ip = item;
		} else {
			new_item->name = item;
		}
	}
	new_item->update();
	_list.push_front(new_item);
}

void HostList::append(HostItem *item) {
	item->update();
	_list.push_front(item);
}

HostList::~HostList() {
	std::string str;
	for(List::reverse_iterator i = _list.rbegin(); i != _list.rend(); ++i) {
		const HostItem *l = dynamic_cast<const HostItem *>(*i);
		if (l == NULL) 
			continue;
		//LOG_DEBUG(("host: %s", l->get().c_str()));
		str += l->ip + "/" + l->name + " ";
	}
	if (!str.empty())
		str.resize(str.size() - 1);
	
	Config->set(_config_key, str);
}
