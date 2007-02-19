#include "upper_box.h"
#include "resource_manager.h"
#include "config.h"
#include "sdlx/surface.h"

void UpperBox::init(int w, int h, const bool server) {
	_server = server;
	_checkbox = ResourceManager->loadSurface("menu/radio.png");
	if (_server) {
		Config->get("multiplayer.game-type", value, "deathmatch");
	} else {
		Config->get("multiplayer.recent-host", value, "localhost");
	}
	Box::init("menu/background_box.png", w, h);

	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_medium.load(data_dir + "/font/medium.png", sdlx::Font::AZ09);
	_big.load(data_dir + "/font/big.png", sdlx::Font::Ascii);
}

void UpperBox::render(sdlx::Surface &surface, const int x, const int y) {
	Box::render(surface, x, y);
	
	int font_dy = (_big.getHeight() - _medium.getHeight()) / 2;
	
	int wt = 0;
	if (_server) {
		wt = _big.render(surface, x + 16, y + 16, "MODE");
	}
	_medium.render(surface, x + (w - wt - 32) / 2, y + 16 + font_dy, value);
	
	int line2_y = 46;
	
	wt = _big.render(surface, x + 16, y + line2_y, "SPLIT SCREEN");
	wt += 96;
	
	int cw = _checkbox->getWidth() / 2;
	
	sdlx::Rect off(0, 0, cw, _checkbox->getHeight());
	sdlx::Rect on(cw, 0, _checkbox->getWidth(), _checkbox->getHeight());
	
	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);
	
	_off_area.x = wt;
	_off_area.y = line2_y;
	_off_area.w = wt;
	_on_area.h = _off_area.h = 32;
	
	surface.copyFrom(*_checkbox, split?off:on, x + wt, y + line2_y);
	wt += cw;
	wt += 16 + _medium.render(surface, x + wt, y + line2_y + font_dy - 2, "OFF");
	_off_area.w = wt - _off_area.w + 1;

	_on_area.x = wt;
	_on_area.y = line2_y;
	_on_area.w = wt;
	surface.copyFrom(*_checkbox, split?on:off, x + wt, y + line2_y);
	wt += cw;
	wt += 16 + _medium.render(surface, x + wt, y + line2_y + font_dy - 2, "ON");
	_on_area.w = wt - _on_area.w + 1;
}

bool UpperBox::onKey(const SDL_keysym sym) {
	return false;
}

bool UpperBox::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (_on_area.in(x, y)) {
		//LOG_DEBUG(("split screen on!"));
		Config->set("multiplayer.split-screen-mode", true);
		return true;
	} else if (_off_area.in(x, y)) {
		//LOG_DEBUG(("split screen off!"));
		Config->set("multiplayer.split-screen-mode", false);
		return true;
	}
	return false;
}
