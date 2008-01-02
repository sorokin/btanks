#include "module.h"
#include "sdlx.h"
#include "sdl_ex.h"

using namespace sdlx;

const std::string Module::mangle(const std::string &name) {
#ifdef _WINDOWS
	return name + ".dll";	
#else 
	return "lib" + name + ".so";
#endif
}

Module::Module() : handle(NULL) {}

void Module::load(const std::string &name) {
	unload();
	handle = SDL_LoadObject(name.c_str());
	if (handle == NULL)
		throw_sdl(("SDL_LoadObject('%s')", name.c_str()));
}

void *Module::sym(const std::string &name) const {
	if (handle == NULL)
		return NULL;
	return SDL_LoadFunction(handle, name.c_str());
}

void Module::unload() {
	if (handle == NULL)
		return;
	SDL_UnloadObject(handle);
	handle = NULL;
}

Module::~Module() {
	unload();
}

void Module::leak() {
	handle = NULL;
}
