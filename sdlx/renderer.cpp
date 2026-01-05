#include "renderer.h"
#include "sdl_ex.h"

using namespace sdlx;

Renderer::Renderer(Renderer&& other)
	: r(std::exchange(other.r, NULL))
{}

Renderer::Renderer(SDL_Window* window, int index, uint32_t flags)
	: r(SDL_CreateRenderer(window, index, flags))
{
	if (r == NULL) throw_sdl(("SDL_CreateRenderer()"));
}

Renderer::~Renderer()
{
	destroy();
}

Renderer& Renderer::operator=(Renderer&& other)
{
	destroy();
	r = std::exchange(other.r, NULL);
}

void Renderer::destroy()
{
	if (r != NULL)
	{
		SDL_DestroyRenderer(r);
		r = NULL;
	}
}

void Renderer::present()
{
	SDL_RenderPresent(r);
}
