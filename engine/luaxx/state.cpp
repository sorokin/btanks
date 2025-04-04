#include "luaxx/state.h"
#include "mrt/exception.h"
#include <stdlib.h>
#include <lauxlib.h>
#include <assert.h>
#include "mrt/chunk.h"

using namespace luaxx;
/*
static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
	if (nsize == 0) {
		free(ptr);
		return NULL;
	} else {
		 return realloc(ptr, nsize);
	}
}
*/

static void check_error(lua_State * state, const int err) {
	switch(err) {
		case 0: 
			return;
		case LUA_ERRRUN:
		case LUA_ERRERR:
		case LUA_ERRSYNTAX: {
			std::string error = lua_tostring(state, -1);
			lua_pop(state, 1);
			throw_ex(("lua error[%d]: %s", err, error.c_str()));
			}
		case LUA_ERRMEM:
			throw_ex(("lua is out of memory"));
		default: 
			throw_ex(("unknown lua error[%d]", err));
	}
}

struct reader_state {
	const mrt::Chunk &data;
	size_t pos;
	reader_state(const mrt::Chunk &data) :data(data), pos(0) {}
};


static const char * chunk_reader(lua_State *L, void *data, size_t *size) {
	assert(size != NULL);
	reader_state * x = (reader_state *) data;
	if (x->pos < x->data.get_size()) {
		*size = x->data.get_size();
		const char *ptr =  (const char *)x->data.get_ptr() + x->pos;
		x->pos += *size;
		return ptr;
	}
	*size = 0;
	return NULL;
}


void State::load(const std::string &fname, const mrt::Chunk &data) {
	//throw_ex(("implement me[%s]", fname.c_str()));
	reader_state x(data);
	int err = lua_load(state, chunk_reader, &x, fname.c_str(), nullptr);
	check_error(state, err);
}


void State::loadFile(const std::string &fname) {
	int err = luaL_loadfile(state, fname.c_str());
	if (err == LUA_ERRFILE)
		throw_ex(("file '%s' not found", fname.c_str()));
	check_error(state, err);
}

void State::call(const int nargs, const int nresults) const {
	int err = lua_pcall(state, nargs, nresults, 0);
	//if (err == LUA_ERRRUN);
	check_error(state, err);
}

void State::init() {
	assert(state == NULL);

	state = luaL_newstate();
	if (state == NULL)
		throw_ex(("cannot create lua interpreter"));

	static const luaL_Reg libs[] = {
		{"", luaopen_base},
		//{LUA_LOADLIBNAME, luaopen_package},
		{LUA_TABLIBNAME, luaopen_table},
		//{LUA_IOLIBNAME, luaopen_io},
		//{LUA_OSLIBNAME, luaopen_os},
		{LUA_STRLIBNAME, luaopen_string},
		{LUA_MATHLIBNAME, luaopen_math},
		 //  {LUA_DBLIBNAME, luaopen_debug},
		{NULL, NULL}
	};
	
	for (const luaL_Reg *lib = libs; lib->func; ++lib) {
		lua_pushcfunction(state, lib->func);
		lua_pushstring(state, lib->name);
		int err = lua_pcall(state, 1, 0, 0);
		check_error(state, err);
	}	
}

State::State() : state(NULL) {
	init();
}

void State::clear() {
	lua_close(state);

	state = NULL;
	init();
}

State::~State() {
	if (state != NULL)
		lua_close(state);
}


