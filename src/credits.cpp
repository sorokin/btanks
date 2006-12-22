/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include "credits.h"
#include "config.h"
#include "math/unary.h"
#include "math/minmax.h"

Credits::Credits() : _w(0), _h(0) {
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_font.load(data_dir + "/font/big.png", sdlx::Font::AZ09, false);
	_medium_font.load(data_dir + "/font/medium.png", sdlx::Font::AZ09, false);
	
	int fh = _font.getHeight(), mfh = _medium_font.getHeight();
	
	std::vector<std::string> lines, lines2; 
	lines.push_back("BATTLE TANKS");
	lines.push_back("");
	lines.push_back("PROGRAMMING");
	lines.push_back("VLADIMIR 'WHOOZLE' MENSHAKOV");
	lines.push_back("");
	lines.push_back("GRAPHICS");
	lines.push_back("ALEXANDER 'METHOS' WAGNER");
	lines.push_back("");
	lines.push_back("LEVEL DESIGN");
	lines.push_back("VLADIMIR 'VZ' ZHURAVLEV");
	lines.push_back("");
	lines.push_back("TOOLS");
	lines.push_back("VLADIMIR 'GOLD' GOLDOBIN");
	lines.push_back("");
	
	lines.push_back("GAME DESIGN");
	lines.push_back("NETIVE MEDIA GROUP 2006");

	lines.push_back("");
	lines.push_back("");
	
	lines2.push_back("THE CREDITS HAVE BEEN COMPLETED IN AN ENTIRELY DIFFERENT STYLE");
	lines2.push_back("AT GREAT EXPENSE AND AT THE LAST MINUTE");
	lines2.push_back("BY A TEAM OF FORTY OR FIFTY WELL-TRAINED LLAMAS.");

	_h = fh * lines.size() + mfh * lines2.size();

	//copy-paste ninja was here ;)
	for(std::vector<std::string>::const_iterator i = lines.begin(); i != lines.end(); ++i) {
		if (i->size() * fh > _w)
			_w = i->size() * fh;
	}
	for(std::vector<std::string>::const_iterator i = lines2.begin(); i != lines2.end(); ++i) {
		if (i->size() * mfh > _w)
			_w = i->size() * mfh;
	}
	_surface.createRGB(_w, _h, 24);
	_surface.convertAlpha();
	
	LOG_DEBUG(("credits %dx%d", _w, _h));
	
	for(size_t i = 0; i < lines.size(); ++i) {	
		const std::string &str = lines[i];
		_font.render(_surface, (_w - str.size() * fh) / 2, i * fh, str);
	}
	for(size_t i = 0; i < lines2.size(); ++i) {
		const std::string &str = lines2[i];
		_medium_font.render(_surface, (_w - str.size() * mfh) / 2, lines.size() * fh + i * mfh, str);
	}
	//copy-paste ninjas have done its evil deed and vanishes.
	_velocity.x = 2;
	_velocity.y = 3;
	_velocity.normalize();
}

void Credits::render(const float dt, sdlx::Surface &surface) {
	_position += _velocity * dt * 150;
	int xmargin = math::max((int)_w - surface.getWidth(), 96);
	int ymargin = math::max((int)_h - surface.getHeight(), 96);
	
	if (_position.x < -xmargin)
		_velocity.x = math::abs(_velocity.x);
	if (_position.x + _w > surface.getWidth() + xmargin)
		_velocity.x = - math::abs(_velocity.x);

	if (_position.y < -ymargin)
		_velocity.y = math::abs(_velocity.y);
	if (_position.y + _h > surface.getHeight() + ymargin)
		_velocity.y = -math::abs(_velocity.y);
	
	surface.copyFrom(_surface, (int)_position.x, (int)_position.y);
}

Credits::~Credits() {}
