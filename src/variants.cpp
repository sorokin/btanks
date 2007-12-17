#include "variants.h"
#include "mrt/serializator.h"
#include "mrt/exception.h"

Variants::Variants() : vars() {}
Variants::Variants(const std::set<std::string> &vars) : vars(vars) {}

const std::string Variants::parse(const std::string &name) {
	vars.clear();
	
	std::string result, in(name);
	size_t pos1;
	while(!in.empty() && (pos1 = in.find('(')) != in.npos) {
		result += in.substr(0, pos1);
		in = in.substr(pos1 + 1);
		size_t pos2 = in.find(')');
		if (pos2 == in.npos)
			throw_ex(("found orphaned '(' at position %u. object: '%s'", (unsigned)pos1, name.c_str()));
		std::string var = in.substr(0, pos2);
		if (var.empty())
			throw_ex(("empty variant found at position %u. object: '%s'", (unsigned)pos1, name.c_str()));
		vars.insert(var);
		in = in.substr(pos2 + 1);
	}
	result += in;
	return result;
}

const std::string Variants::strip(const std::string &name) {
	std::string result, in(name);
	size_t pos1;
	while(!in.empty() && (pos1 = in.find('(')) != in.npos) {
		result += in.substr(0, pos1);
		in = in.substr(pos1 + 1);
		size_t pos2 = in.find(')');
		if (pos2 == in.npos)
			throw_ex(("found orphaned '(' at position %u. object: '%s'", (unsigned)pos1, name.c_str()));
		std::string var = in.substr(0, pos2);
		if (var.empty())
			throw_ex(("empty variant found at position %u. object: '%s'", (unsigned)pos1, name.c_str()));
		//vars.insert(var);
		in = in.substr(pos2 + 1);
	}
	result += in;
	return result;
}


void Variants::update(const Variants &other, const bool remove_old) {
	if (remove_old)
		vars.clear();
	
	for(std::set<std::string>::const_iterator i = other.vars.begin(); i != other.vars.end(); ++i) 
		vars.insert(*i);
}

const std::string Variants::dump() const {
	std::string result;
	for(std::set<std::string>::const_iterator i = vars.begin(); i != vars.end(); ++i) {
		result += '(';
		result += *i;
		result += ')';
	}
	return result;
}

const bool Variants::same(const Variants &other) const {
	std::set<std::string>::const_iterator i = vars.begin(), j = other.vars.begin();
	while(i != vars.end() && j != other.vars.end()) {
		const std::string l = *i, r = *j;
		if (l == r) 
			return true;
		
		if (l < r) {
			++i;
		} else {
			++j;
		}
	}
	return false;
}

const bool Variants::has(const std::string &name) const {
	return vars.find(name) != vars.end();
}

void Variants::add(const std::string &name) {
	vars.insert(name);
}

void Variants::remove(const std::string &name) {
	vars.erase(name);
}


void Variants::serialize(mrt::Serializator &s) const {
	s.add(vars);
}

void Variants::deserialize(const mrt::Serializator &s) {
	s.get(vars);
}
