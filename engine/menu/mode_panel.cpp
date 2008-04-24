#include "mode_panel.h"
#include "i18n.h"
#include "map_desc.h"
#include "chooser.h"
#include "checkbox.h"
#include "box.h"
#include "config.h"
#include "label.h"
#include "grid.h"

ModePanel::ModePanel(const int width) : enable_ctf(false) {
	_time_limits.insert(std::pair<const int, std::string>(0,   "-:--"));
	_time_limits.insert(std::pair<const int, std::string>(60,  "1:00"));
	_time_limits.insert(std::pair<const int, std::string>(90,  "1:30"));
	_time_limits.insert(std::pair<const int, std::string>(120, "2:00"));
	_time_limits.insert(std::pair<const int, std::string>(180, "3:00"));
	_time_limits.insert(std::pair<const int, std::string>(300, "5:00"));
	_time_limits.insert(std::pair<const int, std::string>(420, "7:00"));
	_time_limits.insert(std::pair<const int, std::string>(600, "9:99"));

		add(0, 0, _background = new Box("menu/background_box.png", width, 80));
		
		int w, h;
		getSize(w, h);
		int mx, my;
		_background->getMargins(mx, my);

		std::vector<std::string> values;

		int tl, pos = 0, idx = 0;
		Config->get("multiplayer.time-limit", tl, 300);

		for(TimeLimits::const_iterator i = _time_limits.begin(); i != _time_limits.end(); ++i, ++idx) {
			values.push_back(i->second);
			if (i->first <= tl)
				pos = idx;
		}
		
		Grid *grid = new Grid(4, 2);
		add(mx, my, grid);
		
		_time_limit = new Chooser("big", values);
		_time_limit->set(pos);
		grid->set(0, 0, _time_limit, Grid::Middle | Grid::Center);
		grid->set(0, 1, _tl_label = new Label("small", I18n->get("menu", "time-limit")), Grid::Middle);
		
		bool rr;
		Config->get("multiplayer.random-respawn", rr, false);

		grid->set(1, 0, _random_respawn = new Checkbox(rr), Grid::Middle | Grid::Center);
		grid->set(1, 1, _rr_label = new Label("small", I18n->get("menu", "random-respawn")), Grid::Middle);
		
		std::vector<std::string> teams;
		teams.push_back("0");
		teams.push_back("2");
		teams.push_back("3");
		teams.push_back("4");
		grid->set(0, 2, _teams = new Chooser("big", teams, "menu/teams.png"), Grid::Middle | Grid::Center);
		grid->set(0, 3, _teams_label = new Label("small", I18n->get("menu", "teams")), Grid::Middle);

		bool ctf;
		Config->get("multiplayer.capture-the-flag", ctf, false);
		grid->set(1, 2, _ctf = new Checkbox(ctf), Grid::Middle | Grid::Center);
		grid->set(1, 3, _ctf_label = new Label("small", I18n->get("menu", "capture-the-flag")), Grid::Middle);
		
		grid->set_spacing(5);
		grid->recalculate(0, h - 2 * my);

	validate();
}

void ModePanel::set(const MapDesc &map) {
	hide(map.game_type != GameTypeDeathMatch);
	enable_ctf = map.supports_ctf;
	//LOG_DEBUG(("ctf supported: %s", enable_ctf?"yes":"no"));
	validate();
}

void ModePanel::tick(const float dt) {
	//LOG_DEBUG(("tick(%g)", dt));
	Container::tick(dt);
	if (_time_limit->changed()) {
		_time_limit->reset();
		int idx = _time_limit->get();
		if (idx >= 0) {
			assert(idx < (int)_time_limits.size());
			TimeLimits::const_iterator i;
			for (i = _time_limits.begin(); idx-- && i != _time_limits.end(); ++i);
			assert(i != _time_limits.end());
			Config->set("multiplayer.time-limit", i->first);
		}
	}
	if (_random_respawn->changed()) {
		_random_respawn->reset();
		Config->set("multiplayer.random-respawn", _random_respawn->get());
	}
	if (_ctf->changed()) {
		_ctf->reset();
		bool ctf = _ctf->get();
		Config->set("multiplayer.capture-the-flag", ctf);
		validate();
	}
	if (_teams->changed()) {
		_teams->reset();
		bool ctf = enable_ctf && _ctf->get();
		if (!ctf)
			Config->set("multiplayer.teams", (int)atoi(_teams->getValue().c_str()));
	}
}

void ModePanel::validate() {
	_ctf_label->hide(!enable_ctf);
	_ctf->hide(!enable_ctf);
	
	bool ctf = enable_ctf && _ctf->get();

	_random_respawn->hide(ctf);
	_rr_label->hide(ctf);
	
	//_teams->hide(!ctf);
	//_teams_label->hide(!ctf);	
	if (ctf) {
		for(int i = 0; i < _teams->size(); ++i) 
			_teams->disable(i, i != 1);
		_teams->reset();
	} else {
		int t;
		Config->get("multiplayer.teams", t, 0);
		for(int i = 0; i < _teams->size(); ++i) 
			_teams->disable(i, false);
		try {
			_teams->set(mrt::formatString("%d", t));
		} CATCH("set", );
	}
}
