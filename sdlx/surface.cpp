
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "surface.h"
#include "sdl_ex.h"
#include "SDL/SDL_rwops.h"
#include "mrt/chunk.h"

using namespace sdlx;

int Surface::default_flags  = Default;

void Surface::setDefaultFlags(const Uint32 flags) {
	if (flags == Default)
		throw_ex(("setDefaultFlags doesnt accept 'Default' argument"));
	default_flags = flags;
}


Surface::Surface():surface(NULL) {}
Surface::Surface(SDL_Surface *x) : surface(x) {}

void Surface::assign(SDL_Surface *x) {
	free();
	surface = x;
}


void Surface::getVideo() {
    free();
    surface = SDL_GetVideoSurface();
}

void Surface::createRGB(int width, int height, int depth, Uint32 flags) {
	free();
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));

	Uint32 rmask=0, gmask=0, bmask=0, amask=0;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	surface = SDL_CreateRGBSurface(flags, width, height, depth,
								   rmask, gmask, bmask, amask);
	if(surface == NULL) throw_sdl(("SDL_CreateRGBSurface(%d, %d, %d)", width, height, depth));
}

void Surface::createRGBFrom(void *pixels, int width, int height, int depth,  int pitch) {
	free();

	Uint32 rmask=0, gmask=0, bmask=0, amask=0; //evil example from sdl doc ;(
#if 0
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
#endif
	if (pitch == -1) pitch = width;

	surface = SDL_CreateRGBSurfaceFrom(pixels, width, height, depth, pitch,
									   rmask, gmask, bmask, amask);
	if(surface == NULL) 
		throw_sdl(("SDL_CreateRGBSurface"));

}

void Surface::convert(Surface &dest, PixelFormat *fmt, Uint32 flags)  const {
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));

	SDL_Surface *x = SDL_ConvertSurface(surface, fmt, flags);
	if (x == NULL) 
		throw_sdl(("SDL_ConvertSurface"));
	dest.assign(x);
}

void Surface::convert(Uint32 flags) {
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));

	SDL_Surface *x = SDL_ConvertSurface(surface, surface->format, flags);
	if (x == NULL) 
		throw_sdl(("SDL_ConvertSurface"));
	assign(x);
}



void Surface::setVideoMode(int w, int h, int bpp, int flags) {
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));
    free();
    if ((surface = SDL_SetVideoMode(w, h, bpp, flags)) == NULL ) 
		throw_sdl(("SDL_SetVideoMode(%d, %d, %d, %x)", w, h, bpp, flags));
}


void Surface::putPixel(int x, int y, Uint32 pixel) {
	assert(x >= 0 && y >= 0 && x < surface->w && y < surface->h);
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
	case 1:
		*p = pixel;
		break;

	case 2:
		*(Uint16 *)p = pixel;
		break;

	case 3:
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		p[0] = (pixel >> 16) & 0xff;
		p[1] = (pixel >> 8) & 0xff;
		p[2] = pixel & 0xff;
#else
		p[0] = pixel & 0xff;
		p[1] = (pixel >> 8) & 0xff;
		p[2] = (pixel >> 16) & 0xff;
#endif
		break;

	case 4:
		*(Uint32 *)p = pixel;
		break;

	default:
		throw_ex(("surface has unusual BytesPP value (%d)", bpp));
	}
}

Uint32 Surface::getPixel(int x, int y) const{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
	case 1:
		return *p;

	case 2:
		return *(Uint16 *)p;

	case 3:
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		return p[0] << 16 | p[1] << 8 | p[2];
#else
		return p[0] | p[1] << 8 | p[2] << 16;
#endif

	case 4:
		return *(Uint32 *)p;

	default:
		throw_ex(("surface has unusual BytesPP value (%d)", bpp));
	}
}



void Surface::saveBMP(const std::string &fname)  const {
	if (SDL_SaveBMP(surface, fname.c_str()) == -1) 
		throw_sdl(("SDL_SaveBMP"));
}


#ifndef NO_SDL_IMAGE
#include <SDL/SDL_image.h>
void Surface::loadImage(const std::string &str) {
    free();
    if ((surface = IMG_Load(str.c_str())) == NULL ) throw_sdl(("IMG_Load"));
}

void Surface::loadImage(const mrt::Chunk &memory) {
	free();
	SDL_RWops *op = SDL_RWFromMem(memory.getPtr(), memory.getSize());
	if (op == NULL) throw_sdl(("SDL_RWFromMem"));
	try {
		surface = IMG_Load_RW(op, 0);
		SDL_FreeRW(op);
		op = NULL;
		if (surface == NULL)
			throw_sdl(("IMG_Load_RW"));
	} CATCH("loadImage", {SDL_FreeRW(op); throw;})
}

#endif


void Surface::copyFrom(const Surface &from, const int x, const int y) {
    SDL_Rect dst;
	memset(&dst, 0, sizeof(dst));
    dst.x = x;
    dst.y = y;
    if (SDL_BlitSurface(from.surface, NULL, surface, &dst) == -1) 
		throw_sdl(("SDL_BlitSurface"));
}

void Surface::copyFrom(const Surface &from, const Rect &fromRect, const int x, const int y) {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    if (SDL_BlitSurface(from.surface, const_cast<Rect*>(&fromRect), surface, &dst) == -1) throw_sdl(("SDL_BlitSurface"));
}

void Surface::copyFrom(const Surface &from, const Rect &fromRect) {
    if (SDL_BlitSurface(from.surface, const_cast<Rect*>(&fromRect), surface, NULL) == -1) throw_sdl(("SDL_BlitSurface"));
}

void Surface::update(const Rect &rect) {
    SDL_UpdateRect(surface, rect.x, rect.y, rect.w, rect.h);
}

void Surface::update() {
    SDL_UpdateRect(surface, 0, 0, 0, 0);
}

void Surface::update(const int x, const int y, const int w, const int h) {
    SDL_UpdateRect(surface, x, y, w, h);
}

void Surface::flip() {
	//SDL_Flip(surface);
	if ((surface->flags & SDL_OPENGL) == SDL_OPENGL) {
		SDL_GL_SwapBuffers();
	} else {
		if (SDL_Flip(surface) == -1)
			throw_sdl(("SDL_Flip"));
	}
}

void Surface::toggleFullscreen() {
	if (SDL_WM_ToggleFullScreen(surface) != 1) 
		throw_sdl(("SDL_WM_ToggleFullScreen"));
}

void Surface::fillRect(const Rect &r, Uint32 color) {
    if ( SDL_FillRect(surface, (SDL_Rect *)&r , color) == -1) throw_sdl(("SDL_FillRect"));
}


void Surface::setAlpha(Uint8 alpha, Uint32 flags) {
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));

    if (SDL_SetAlpha(surface, flags, alpha) == -1) throw_sdl(("SDL_SetAlpha"));
}

void Surface::convertAlpha() {
	SDL_Surface *r = SDL_DisplayFormatAlpha(surface);
	if (r == NULL)
		throw_sdl(("SDL_DisplayFormatAlpha"));
	assign(r);
}

void Surface::convertToDisplay() {
	SDL_Surface *r = SDL_DisplayFormat(surface);
	if (r == NULL)
		throw_sdl(("SDL_DisplayFormat"));
	assign(r);
}


void Surface::free() {
    if (surface == NULL) return;
    SDL_FreeSurface(surface);
    surface = NULL;
}

void Surface::lock() const {
	if (SDL_MUSTLOCK(surface)) {
		if (SDL_LockSurface(surface) == -1) 
			throw_sdl(("SDL_LockSurface"));
	}
}

void Surface::unlock() const {
	if (SDL_MUSTLOCK(surface)) {
		SDL_UnlockSurface(surface);
	}
}

void Surface::convertToHardware() {
	if ((surface->flags & SDL_HWSURFACE) == SDL_HWSURFACE) {
		LOG_DEBUG(("%p is already in hardware, skipping", (void*) surface));
	}
	convert((surface->flags & ~SDL_SWSURFACE) | SDL_HWSURFACE);
	//LOG_DEBUG(("moving %p to hardware,  result: %s", (void *)surface, ((surface->flags & SDL_HWSURFACE) == SDL_HWSURFACE)?"hardware":"software"));
}

void Surface::setClipRect(const sdlx::Rect &rect) {
	SDL_SetClipRect(surface, const_cast<sdlx::Rect*>(&rect));
}
void Surface::resetClipRect() {
	SDL_SetClipRect(surface, NULL);
}
void Surface::getClipRect(sdlx::Rect &rect) {
	SDL_GetClipRect(surface, &rect);
}

Surface::~Surface() {
    free();
}

