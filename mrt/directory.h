#ifndef __MRT_DIRECTORY_H__
#define __MRT_DIRECTORY_H__

#ifndef WIN32
#	include <sys/types.h>
#	include <dirent.h>
#endif
#include <string>
#include "fs_node.h"
#include "export_mrt.h"

namespace mrt { 

class MRTAPI Directory : public FSNode {
public: 

	Directory();
	void open(const std::string &path);
	const bool opened() const;
	const std::string read() const;
	void close();
	~Directory();
	
	static const std::string getHome();
	static const std::string getAppDir(const std::string &name, const std::string &shortname);
	static void create(const std::string &path);

private: 


#ifdef WIN32
	typedef long dir_t;
	mutable std::string _first_value;
#else
	typedef DIR * dir_t;
#endif

	dir_t _handle;
};
}

#endif

