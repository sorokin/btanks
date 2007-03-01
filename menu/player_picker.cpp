#include "player_picker.h"
#include "container.h"
#include "sdlx/font.h"
#include "resource_manager.h"
#include "chooser.h"
#include "label.h"
#include "map_desc.h"
#include "menu_config.h"
#include "config.h"

class SlotLine : public Container {
public : 

	Chooser *_type, *_vehicle;

	int h, ch;
	std::string map, variant;
	int slot;
	SlotConfig config;
	
	SlotLine(const std::string &map, const std::string &variant, const int i, const SlotConfig &config) : 
	map(map), variant(variant), slot(i), config(config) {
		
		_font = ResourceManager->loadFont("medium", true);
		h = _font->getHeight();
		int w = _font->getWidth();

		std::vector<std::string> options;
		options.push_back("?");

		if (variant =="split") {
			options.push_back("PLAYER-1");
			options.push_back("PLAYER-2");			
			options.push_back("AI");
		} else {
			options.push_back("PLAYER");
			options.push_back("AI");
		}

		_type = new Chooser("medium", options);
		if(!config.type.empty())
			_type->set(config.type);
			
		options.resize(1);
		options.push_back("tank");
		options.push_back("launcher");
		options.push_back("shilka");
		options.push_back("machinegunner");
		
		_vehicle = new Chooser("medium", options, "menu/vehicles.png");
		if(!config.vehicle.empty())
			_vehicle->set(config.vehicle);
		
		int cw;
		_type->getSize(cw, ch);

		add(sdlx::Rect(0, (ch - h) / 3, w, h), new Label(_font, mrt::formatString("%d", i + 1)));


		sdlx::Rect p1;
		p1.x = w * 2;
		//p1.y = (_font->getHeight() - ch) / 2;
		p1.w = cw;
		p1.h = ch;
		if (ch > h) 
			h = ch;
		
		add(p1, _type);
		
		sdlx::Rect p2;
		p2.x = p1.x + p1.w + _font->getWidth();

		int vcw, vch;
		_vehicle->getSize(vcw, vch);
		if (vch > h) 
			h = vch;
		p2.w = vcw; p2.h = vch;

		add(p2, _vehicle);
	}

	void tick(const float dt) {
		if (_type->changed()) {
			_type->reset();
			config.type = _type->getValue();
			_changed = true;
			//LOG_DEBUG(("type changed"));
		}
		if (_vehicle->changed()) {
			_vehicle->reset();
			config.vehicle = _vehicle->getValue();
			_changed = true;
			//LOG_DEBUG(("vehicle changed"));
		}

		if (_changed)
			MenuConfig->update(map, variant, slot, config);
	}

private: 
	const sdlx::Font *_font;
};

PlayerPicker::PlayerPicker(const int w, const int h)  {
	_background.init("menu/background_box.png", w, h);
	_vehicles = ResourceManager->loadSurface("menu/vehicles.png");
}

const std::string PlayerPicker::getVariant() const {
	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);
	return split?"split":std::string();
}

const bool PlayerPicker::validateSlots(const int changed) {
	const std::string variant = getVariant();
	if (variant == "split") {
		int p1 = 0, p2 = 0;
		bool changed_p1 =  _slots[changed]->config.hasType("player-1");
		bool changed_p2 =  _slots[changed]->config.hasType("player-2");

		for(size_t i = 0; i < _slots.size(); ++i) {
			SlotLine *slot = _slots[i];
			if (slot->config.hasType("player-1"))
				++p1;
			if (slot->config.hasType("player-2"))
				++p2;
		}

		if (p1 == 1 && p2 == 1) 
			return false;
		
		if (p1 > 1) {
			if (changed_p1) 
				changeSlotTypesExcept("player-1", "ai", changed, 0);
			else 
				changeSlotTypesExcept("player-1", "ai", -1, 1);
		}

		if (p2 > 1) {
			if (changed_p2) 
				changeSlotTypesExcept("player-2", "ai", changed, 0);
			else 
				changeSlotTypesExcept("player-2", "ai", -1, 1);
		}

		if (p1 > 1 || p2 > 1)
			return true;

		if (p1 == 0) {
			if (!changeAnySlotTypeExcept("ai", "player-1", changed))
				changeAnySlotTypeExcept("?", "player-1", changed);
		}

		if (p2 == 0) {
			if (!changeAnySlotTypeExcept("ai", "player-2", changed))
				changeAnySlotTypeExcept("?", "player-2", changed);
		}
	} else {
		int p1 = 0;
		bool changed_p1 =  _slots[changed]->config.hasType("player");
		
		for(size_t i = 0; i < _slots.size(); ++i) {
			SlotLine *slot = _slots[i];
			if (slot->config.hasType("player"))
				++p1;
		}
		if (p1 == 0) {
			if (!changeAnySlotTypeExcept("ai", "player", changed))
				changeAnySlotTypeExcept("?", "player", changed);
		} else if (p1 > 1) {
			if (changed_p1) 
				changeSlotTypesExcept("player", "ai", changed, 0);
			else 
				changeSlotTypesExcept("player", "ai", -1, 1);
		}
	}

	return false;
}

const bool PlayerPicker::changeAnySlotTypeExcept(const std::string &what, const std::string &to, const int e) {
	for(int i = 0; i < (int)_slots.size(); ++i) {
		if (i == e) 
			continue;
		
		SlotLine *slot = _slots[i];
		if (slot->config.hasType(what)) {
			slot->_type->set(to);
			return true;
		}
	}
	return false;
}


const bool PlayerPicker::changeSlotTypesExcept(const std::string &what, const std::string &to, const int e, const int skip) {
	int s = skip;
	
	bool found = false;
	for(int i = 0; i < (int)_slots.size(); ++i) {
		if (skip == 0 && i == e) 
			continue;

		if (s && s--) 
			continue;
		
		
		SlotLine *slot = _slots[i];
		if (slot->config.hasType(what)) {
			slot->_type->set(to);
			found = true;
		}
	}
	return found;	
}

void PlayerPicker::tick(const float dt) {
	for(size_t i = 0; i < _slots.size(); ++i) {
		SlotLine *slot = _slots[i];
		if (slot->changed()) {
			slot->reset();
			validateSlots(i);
		}
	}
	Container::tick(dt);
}

void PlayerPicker::set(const MapDesc &map) {
	clear();
	int mx, my;
	_background.getMargins(mx, my);
	_object = map.object;

	std::vector<SlotConfig> config;

	std::string variant = getVariant();
	
	MenuConfig->fill(map.name, variant, config);
	config.resize(map.slots);

	_slots.clear();
	
	for(int i = 0; i < map.slots; ++i) {
		SlotLine *line = new SlotLine(map.name, variant, i, config[i]);
		_slots.push_back(line);
		sdlx::Rect pos(mx, my + i * (line->h + 6), _background.w - 2 * mx, line->h);
		add(pos, line);
	}
}

void PlayerPicker::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	
	int mx, my;
	_background.getMargins(mx, my);
	Container::render(surface, x, y);
}

bool PlayerPicker::onKey(const SDL_keysym sym) {
	return Container::onKey(sym);
}

bool PlayerPicker::onMouse(const int button, const bool pressed, const int x, const int y)  {
	return Container::onMouse(button, pressed, x, y);
}
