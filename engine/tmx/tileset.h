#ifndef BTANKS_TILESET_H__
#define BTANKS_TILESET_H__

#include "xml_parser.h"
#include <deque>
#include <string>
#include <map>
#include <memory>

class GeneratorObject;
class Tileset : public XMLParser {
public: 
	Tileset();

	const GeneratorObject *getObject(const std::string &name) const;
	void getPrimaryBoxes(std::deque<std::string> &boxes);
	~Tileset();

	Tileset(Tileset const& other) = delete;
	Tileset& operator=(Tileset const& other) = delete;

private: 
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void cdata(const std::string &data);

	Attrs  _attr;
	std::string _cdata;

	typedef std::map<std::string, std::unique_ptr<GeneratorObject>> Objects;
	Objects _objects;
};

#endif

