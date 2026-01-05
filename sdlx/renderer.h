#ifndef __SDLX_RENDERER_H__
#define __SDLX_RENDERER_H__

#include "sdlx.h"

namespace sdlx {
	class SDLXAPI Renderer {
	public:
		Renderer() = default;
		Renderer(Renderer&& other);
		Renderer(SDL_Window* window, int index, uint32_t flags);
		~Renderer();

		Renderer& operator=(Renderer&& other);

		void destroy();
		void present();

	private:
		Renderer(const Renderer &x) = delete;
		Renderer& operator=(const Renderer &x) = delete;

		SDL_Renderer* r = NULL;
	};
}

#endif
