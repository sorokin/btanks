#include "registrar.h"
#include "object.h"
#include "resource_manager.h"

void Registrar::registerObject(const std::string &name, std::unique_ptr<Object> object) {
	ResourceManager->registerObject(name, std::move(object));
}
