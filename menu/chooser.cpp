#include "chooser.h"
#include "resource_manager.h"
#include "math/binary.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"

#ifdef WIN32
#	define strcasecmp _stricmp
#endif

Chooser::Chooser(const std::string &font, const std::vector<std::string> &options, const std::string &surface, const int tw) : 
_options(options), _i(0), _n(options.size()), _tw(tw), _surface(NULL), _w(0) {
	if (!surface.empty())
		_surface = ResourceManager->loadSurface(surface);
	
	_left_right = ResourceManager->loadSurface("menu/left_right.png");
	_font = ResourceManager->loadFont(font, true);
	for(size_t i =0; i < options.size(); ++i) {
		int w = _font->render(NULL, 0, 0, options[i]);
		if (w > _w)
			_w = w;
	}
}

const std::string& Chooser::getValue() const {
	if (_options.size() == 0)
		throw_ex(("getValue() on non-text Chooser is invalid"));
	return _options[_i];
}


void Chooser::getSize(int &w, int &h) {
	if (_surface != NULL) {
		w = _left_right->getWidth() + _tw;
		h = math::max(_left_right->getHeight(), _surface->getHeight());
	} else {
		w = _left_right->getWidth() + _w;
		h = math::max(_left_right->getHeight(), _font->getHeight());
	}
}

void Chooser::render(sdlx::Surface &surface, const int x, const int y) {
	Container::render(surface, x, y);
	int lrw = _left_right->getWidth() / 2;
	int lrh = _left_right->getHeight();
	
	int w, h;
	getSize(w, h);
	
	_left_area = sdlx::Rect(0, 0, lrw, lrh);
	_right_area = sdlx::Rect(w - lrw, 0, lrw, lrh);
	
	surface.copyFrom(*_left_right, sdlx::Rect(0, 0, lrw, lrh), x + _left_area.x, y + _left_area.y);
	
	if (_surface) {
		surface.copyFrom(*_surface, 
			sdlx::Rect(_i * _tw, 0, _tw, _surface->getHeight()), 
			x + _left_area.x + lrw, y + (_left_area.h - _surface->getHeight())/ 2);
	} else { 
		int tw = _font->render(NULL, 0, 0, _options[_i]);
		_font->render(surface, x + _left_area.x + (w - tw) / 2, y + (_left_area.h - _font->getHeight())/ 2, _options[_i]);
	} 
	
	surface.copyFrom(*_left_right, sdlx::Rect(lrw, 0, lrw, lrh), x +  _right_area.x, y + _right_area.y);
}

bool Chooser::onMouse(const int button, const bool pressed, const int x, const int y) {
	//LOG_DEBUG(("click: %d %d", x, y));
	if (_left_area.in(x, y)) {
		left();
		return true;
	} else if (_right_area.in(x, y)) {
		right();
		return true;
	} 
	return Container::onMouse(button, pressed, x, y);
}

void Chooser::set(const int i) {
	if (i < 0 || i >= _n)
		throw_ex(("set(%d) is greater than available options (%d)", i, _n));
	_i = i;
	_changed = true;
}

void Chooser::set(const std::string &name) {
	for(int i = 0; i < _n; ++i) {
		if (strcasecmp(name.c_str(), _options[i].c_str()) == 0) {
			_i = i;
			_changed = true;
			return;
		}
	}
	throw_ex(("chooser doesnt contain option '%s'", name.c_str()));
}

void Chooser::left() {
	--_i;
	if (_i < 0)
		_i = _n - 1;
	_changed = true;	
}

void Chooser::right() {
	++_i;
	if (_i >= _n)
		_i = 0;
	_changed = true;
}
