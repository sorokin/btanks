#ifndef BTANKS_REGISTRAR_H__
#define BTANKS_REGISTRAR_H__

#include "export_btanks.h"
#include <memory>
#include <string>

class Object;

class BTANKSAPI Registrar {
public: 
static void registerObject(const std::string &name, std::unique_ptr<Object> object);
};

#define CONCATENATE(x, y) CONCATENATE_DIRECT(x, y) 
#define CONCATENATE_DIRECT(x, y) x##y

#define REGISTER_OBJECT(name, classname, args) class CONCATENATE(classname##Registrar, __LINE__) {\
public: \
	CONCATENATE(classname##Registrar, __LINE__)() { TRY { Registrar::registerObject(name, std::make_unique<classname> args); } CATCH("registering class", throw;) } \
} CONCATENATE(instance_of_##classname##Registrar, __LINE__)


#endif
