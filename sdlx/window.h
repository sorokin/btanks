#ifndef __SDLX_WINDOW_H__
#define __SDLX_WINDOW_H__

#include "sdlx.h"

namespace sdlx {
	class SDLXAPI Window {
	public:
		Window() = default;
		Window(Window&& other);
		Window(const char* title, int x, int y, int w, int h, uint32_t flags);
		Window(const char* title, int w, int h, uint32_t flags);
		~Window();

		Window& operator=(Window&& other);

		SDL_Window* get_sdl_window();

		void destroy();
		void set_window_fullscreen(uint32_t flags);

	private:
		Window(const Window &x) = delete;
		Window& operator=(const Window &x) = delete;

		SDL_Window* w = NULL;
	};
}

#endif
