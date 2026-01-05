#include "window.h"
#include "sdl_ex.h"

using namespace sdlx;

Window::Window(Window&& other)
	: w(std::exchange(other.w, NULL))
{}

Window::Window(const char* title, int x, int y, int w, int h, uint32_t flags)
	: w(SDL_CreateWindow(title, x, y, w, h, flags))
{
	if (w == NULL) throw_sdl(("SDL_CreateWindow()"));
}

Window::Window(const char* title, int w, int h, uint32_t flags)
	: Window(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags)
{}

Window::~Window()
{
	destroy();
}

Window& Window::operator=(Window&& other)
{
	destroy();
	w = std::exchange(other.w, NULL);
}

SDL_Window* Window::get_sdl_window()
{
	return w;
}

void Window::destroy()
{
	if (w != NULL)
	{
		SDL_DestroyWindow(w);
		w = NULL;
	}
}

void Window::set_window_fullscreen(uint32_t flags)
{
	int result = SDL_SetWindowFullscreen(w, flags);
	if (result != 0)
		throw_sdl(("SDL_SetWindowFullscreen()"));
}
