#ifndef BTANKS_MENU_MEDALS_H__
#define BTANKS_MENU_MEDALS_H__

#include "container.h"

namespace sdlx {
	class Surface;
}

class Campaign;
class Label;
class Tooltip;
class Image;

class Medals : public Container {
public: 
	Medals(int w, int h); 
	bool onKey(const SDL_keysym sym);
	virtual void hide(const bool hide = true);
	void set(const Campaign * c);
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;

private: 
	void update();
	void get_medals(const std::string &id, int &now, int &total) const;
	
	int _w, _h;

	const Campaign * campaign;
	std::vector<const sdlx::Surface *> tiles;
	int active;
	
	Label * title, *numbers;
	Image * image;
	Tooltip * hint;
};

#endif
