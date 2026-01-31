#include <assert.h>
#include <algorithm>
#include "tileset.h"
#include "generator_object.h"
#include "mrt/random.h"
#include "utils.h"

void Tileset::getPrimaryBoxes(std::deque<std::string> &boxes) {
	boxes.clear();
	for(Objects::const_iterator i = _objects.begin(); i != _objects.end() ; ++i) {
		boxes.push_back(i->first);
	}
}

void Tileset::start(const std::string &name, Attrs &attr) {
	if (name == "tileset")
		return;
	
	if (name == "background" && attr["id"].empty()) {
			throw_ex(("empty id for element %s", name.c_str()));
	}
	if (name == "box") {
		if (attr["in"].empty() && attr["out"].empty())
			throw_ex(("box must provide at least one of 'in'/'out' attrs."));
		attr["id"] = attr["in"] + "|" + attr["out"];
	}
	_cdata.clear();
	_attr = attr;
}

void Tileset::cdata(const std::string &data) {
	_cdata += data;
}

void Tileset::end(const std::string &name) {
	if (name == "tileset")
		return;
		
	if (_objects.find(name) != _objects.end())
		throw_ex(("duplicate id %s", name.c_str()));

	std::string id =  _attr["id"];
	auto o = GeneratorObject::create(name, _attr, _cdata);
	LOG_DEBUG(("adding '%s' object with id '%s' (%p)", name.c_str(), id.c_str(), (void *)o.get()));
	_objects.insert(std::make_pair(id, std::move(o)));
}

Tileset::Tileset()
{}

const GeneratorObject *Tileset::getObject(const std::string &name) const {
	if (name == "?") {
		if (_objects.empty())
			return nullptr;
		
		int n = mrt::random(_objects.size());
		Objects::const_iterator i = _objects.begin();
		while(n--) {
			++i;
		}
		return i->second.get();
	}
	
	Objects::const_iterator i = _objects.find(name);
	if (i == _objects.end())
		return nullptr;

	assert(i->second != nullptr);
	
	return i->second.get();
}

Tileset::~Tileset() {
}
